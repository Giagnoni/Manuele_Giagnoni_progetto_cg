#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>
#include <iostream>

#define NANOSVG_IMPLEMENTATION
#include "3dparty/nanosvg/src/nanosvg.h"
#define NANOSVGRAST_IMPLEMENTATION
#include "3dparty/nanosvg/src/nanosvgrast.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "../common/shaders.h"
#include "../common/simple_shapes.h"
#include "../common/matrix_stack.h"
#include "../common/frame_buffer_object.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define TINYGLTF_IMPLEMENTATION
#include "../common/gltf_loader.h"

#include "../common/carousel/carousel.h"
#include "../common/carousel/carousel_loader.h"
#include "../common/carousel/carousel_to_renderable.h"

#include "../common/intersection.h"
#include "../common/trackball.h"

trackball tb[2];
int curr_tb;

glm::mat4 proj;
glm::mat4 view;
glm::mat4 Lproj;
glm::mat4 Lview;
glm::mat4 Lpv;

race r;
renderable fram, r_cube, r_lamps, r_terrain, r_track, r_trees, r_quad;

matrix_stack stack;

std::vector<stick_object> alberi, lampioni;

shader fsq_shader;

/* callback function called when the mouse is moving */
static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	tb[curr_tb].mouse_move(proj, view, xpos, ypos);
}

/* callback function called when a mouse button is pressed */
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
		tb[curr_tb].mouse_press(proj, view, xpos, ypos);
	}
	else
		if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
			tb[curr_tb].mouse_release();
		}
}

/* callback function called when a mouse wheel is rotated */
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	if (curr_tb == 0)
		tb[0].mouse_scroll(xoffset, yoffset);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	/* every time any key is presse it switch from controlling trackball tb[0] to tb[1] and viceversa */
	if (action == GLFW_PRESS || action == GLFW_REPEAT) {
		switch (key) {
		case GLFW_KEY_S:
			tb[0].fly(glm::vec3(0, 0, 0.01));
			break;
		case GLFW_KEY_W:
			tb[0].fly(glm::vec3(0, 0, -0.01));
			break;
		case GLFW_KEY_A:
			tb[0].fly(glm::vec3(-0.01, 0, 0));
			break;
		case GLFW_KEY_D:
			tb[0].fly(glm::vec3(0.01, 0, 0));
			break;
		}
	}
}

texture texture_terreno, texture_pista;

void caricaTextures() {
	texture_terreno.load("textures/tilable-img_0044-dark.png", 0);
	texture_pista.load("textures/TwoLaneRoadWet01_MR_1K/TwoLaneRoadWet01_1K_BaseColor.png", 0);
}

gltf_loader carica_auto, carica_albero, carica_lampioni;
std::vector<renderable> modelli_auto, modello_albero, modello_lampioni;
box3 box;

void caricaModelli() {
	carica_albero.load_to_renderable("modelli/uploads_files_5194613_tree.gltf", modello_albero, box);
	carica_auto.load_to_renderable("modelli/uploads_files_6173471_MUSCLE_CAR.glb", modelli_auto, box);
	carica_lampioni.load_to_renderable("modelli/StreetLamp2_Tall.glb", modello_lampioni, box);
}

void draw_scene(shader sh) {
	glUseProgram(sh.program);

	fram.bind();
	glDrawArrays(GL_LINES, 0, 6);

	glColor3f(0, 0, 1);
	glBegin(GL_LINES);
	glVertex3f(0, 0, 0);
	glVertex3f(r.sunlight_direction().x, r.sunlight_direction().y, r.sunlight_direction().z);
	glEnd();
	if (sh.has_uniform("uSunlight")) glUniform3fv(sh["uSunlight"], 1, &r.sunlight_direction()[0]);

	float s = 1.f / r.bbox().diagonal();
	glm::vec3 c = r.bbox().center();

	stack.push();
	stack.mult(glm::scale(glm::mat4(1.f), glm::vec3(s)));
	stack.mult(glm::translate(glm::mat4(1.f), -c));

	glDepthRange(0.01, 1);
	glCullFace(GL_FRONT);
	glUniformMatrix4fv(sh["model_matrix"], 1, GL_FALSE, &stack.m()[0][0]);
	if(sh.has_uniform("oggetto_mappato")) glUniform1i(sh["oggetto_mappato"], 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture_terreno.id);
	r_terrain.bind();
	//glDrawArrays(GL_POINTS, 0, r_terrain().count);
	glDrawElements(r_terrain().mode, r_terrain().count, r_terrain().itype, 0);
	glDepthRange(0.0, 1);

	glCullFace(GL_BACK);
	if (sh.has_uniform("oggetto_mappato")) glUniform1i(sh["oggetto_mappato"], 2);
	for (unsigned int ic = 0; ic < r.cars().size(); ++ic) {
		stack.push();
		stack.mult(r.cars()[ic].frame);
		stack.mult(glm::translate(glm::mat4(1.f), glm::vec3(0, 0.1, 0.0)));
		glUniformMatrix4fv(sh["model_matrix"], 1, GL_FALSE, &stack.m()[0][0]);
		//fram.bind();
		//glDrawArrays(GL_LINES, 0, 6);

		for (int i = 0; i < modelli_auto.size(); i++) {
			modelli_auto[i].bind();
			stack.push();
			stack.mult(modelli_auto[i].transform);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, modelli_auto[i].mater.base_color_texture);

			glUniformMatrix4fv(sh["model_matrix"], 1, GL_FALSE, &stack.m()[0][0]);
			glDrawElements(modelli_auto[i]().mode, modelli_auto[i]().count, modelli_auto[i]().itype, 0);
			stack.pop();
		}

		stack.pop();
	}

	for (unsigned int ic = 0; ic < r.cameramen().size(); ++ic) {
		stack.push();
		stack.mult(r.cameramen()[ic].frame);
		stack.mult(glm::scale(glm::mat4(1.f), glm::vec3(4, 4, 4)));
		glUniformMatrix4fv(sh["model_matrix"], 1, GL_FALSE, &stack.m()[0][0]);
		glDrawArrays(GL_LINES, 0, 6);
		stack.pop();
	}

	glUniformMatrix4fv(sh["model_matrix"], 1, GL_FALSE, &stack.m()[0][0]);

	r_track.bind();
	glPointSize(3.0);
	glCullFace(GL_FRONT);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture_pista.id);
	if (sh.has_uniform("oggetto_mappato")) glUniform1i(sh["oggetto_mappato"], 1);
	//glDrawArrays(GL_LINE_STRIP, 0, r_track.vn);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, r_track.vn);
	glPointSize(1.0);


	//r_trees.bind();
	//glDrawArrays(GL_LINES, 0, r_trees.vn);
	glCullFace(GL_BACK);
	if (sh.has_uniform("oggetto_mappato")) glUniform1i(sh["oggetto_mappato"], 2);
	float scala = 1.f / modello_albero[0].bbox.diagonal() * alberi[0].height * 2;
	for (int i = 0; i < alberi.size(); i++) {
		stack.push();

		stack.mult(glm::translate(alberi[i].pos));
		stack.mult(glm::scale(glm::mat4(1.f), glm::vec3(scala, scala, scala)));

		for (int j = 0; j < modello_albero.size(); j++) {
			modello_albero[j].bind();
			stack.push();
			stack.mult(modello_albero[j].transform);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, modello_albero[j].mater.base_color_texture);

			glUniformMatrix4fv(sh["model_matrix"], 1, GL_FALSE, &stack.m()[0][0]);
			glDrawElements(modello_albero[j]().mode, modello_albero[j]().count, modello_albero[j]().itype, 0);
			stack.pop();
		}
		stack.pop();
	}

	//r_lamps.bind();
	//glDrawArrays(GL_LINES, 0, r_lamps.vn);
	scala = 1.f / modello_lampioni[0].bbox.diagonal() * lampioni[0].height * 2;
	if(sh.has_uniform("uColor")) glUniform3f(sh["uColor"], 0.5, 0.5, 0.5);
	for (int i = 0; i < lampioni.size(); i++) {
		stack.push();

		stack.mult(glm::translate(lampioni[i].pos));
		stack.mult(glm::scale(glm::mat4(1.f), glm::vec3(scala, scala, scala)));

		for (int j = 0; j < modello_lampioni.size(); j++) {
			modello_lampioni[j].bind();
			stack.push();
			stack.mult(modello_lampioni[j].transform);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, modello_lampioni[j].mater.base_color_texture);

			glUniformMatrix4fv(sh["model_matrix"], 1, GL_FALSE, &stack.m()[0][0]);
			glDrawElements(modello_lampioni[j]().mode, modello_lampioni[j]().count, modello_lampioni[j]().itype, 0);

			stack.pop();
		}

		stack.pop();
	}
	if (sh.has_uniform("uColor")) glUniform3f(sh["uColor"], -1, 0.5, 0.5);

	stack.pop();
}

void draw_full_screen_quad() {
	r_quad.bind();
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void draw_texture(GLint tex_id) {
	GLint at;
	glGetIntegerv(GL_ACTIVE_TEXTURE, &at);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, tex_id);
	glUseProgram(fsq_shader.program);
	glUniform1i(fsq_shader["uTexture"], 3);
	draw_full_screen_quad();
	glUseProgram(0);
	glActiveTexture(at);
}

int main() {
	if (!glfwInit()) return -1;

	GLFWwindow* window;
	int w = 1024, h = 1024;

	window=glfwCreateWindow(w, h, "Finestra", NULL, NULL);
	if (!window) {
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::fprintf(stderr, "GLAD non caricato");
		glfwTerminate();
		return -1;
	}

	carousel_loader::load("small_test.svg", "terrain_256.png", r);

	for (int i = 0; i < 10; i++) {
		r.add_car();
	}

	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetKeyCallback(window, key_callback);

	fram = shape_maker::frame();
	r_cube = shape_maker::cube();

	r_track.create();
	game_to_renderable::to_track(r, r_track);
	std::vector<float> progressione_pista;
	progressione_pista.resize(r.t().curbs[0].size() * 2 * 2);
	for (float i = 0; i < r.t().curbs[0].size()*4; i+=4) {
		progressione_pista[i] = i;
		progressione_pista[i + 1] = 0;
		progressione_pista[i + 2] = i;
		progressione_pista[i + 3] = 1;
	}
	r_track.add_vertex_attribute<float>(&progressione_pista[0], progressione_pista.size(), 10, 2);

	r_terrain.create();
	game_to_renderable::to_heightfield(r, r_terrain);

	r_trees.create();
	game_to_renderable::to_tree(r, r_trees);

	r_lamps.create();
	game_to_renderable::to_lamps(r, r_lamps);

	r_quad = shape_maker::quad();

	tb[0].reset();
	tb[0].set_center_radius(glm::vec3(0, 0, 0), 1.f);
	curr_tb = 0;

	float smw = w * 2, smh = h * 2;
	frame_buffer_object fbo;
	fbo.create(smw, smh, true);

	r.start(11, 0, 0, 600);
	r.update();

	shader shader_program, depth_mapper;
	shader_program.create_program("shaders/pipeline.vert", "shaders/texture.frag");
	depth_mapper.create_program("shaders/depthmap.vert", "shaders/depthmap.frag");
	fsq_shader.create_program("shaders/fsq.vert", "shaders/fsq.frag");

	float altezza_luce = 2.5f;
	float shadow_frustum_dim = 1.f;

	proj = glm::perspective(glm::radians(45.f), w/float(h), 1.f, 40.f);
	view = glm::lookAt(glm::vec3(0, 1.f, 1.5), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));
	Lproj = glm::ortho(-shadow_frustum_dim, shadow_frustum_dim, -shadow_frustum_dim, shadow_frustum_dim, 0.f, 15.f);
	Lview = glm::lookAt(r.sunlight_direction(), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, -1.f));
	Lpv = Lproj * Lview;


	glUseProgram(shader_program.program);
	glUniformMatrix4fv(shader_program["projection_matrix"], 1, GL_FALSE, &proj[0][0]);
	glUniformMatrix4fv(shader_program["view_matrix"], 1, GL_FALSE, &view[0][0]);
	glUniform1f(shader_program["uBias"], 0.0003);
	glUniform1i(shader_program["uShadowMap"], 1);
	glUniform2i(shader_program["uShadowMapSize"], smw, smh);
	glUniform3f(shader_program["uSunlight"], r.sunlight_direction().x, r.sunlight_direction().y, r.sunlight_direction().z);
	glUniform3f(shader_program["uColor"], -1, 0, 0);
	glUniform1f(shader_program["spotCutoff"], 12);

	glUseProgram(depth_mapper.program);
	glUniform1f(depth_mapper["uPlaneApprox"], 0.5);
	glUniformMatrix4fv(depth_mapper["uLightMatrix"], 1, GL_FALSE, &Lpv[0][0]);

	alberi = r.trees();
	lampioni = r.lamps();

	caricaTextures();
	caricaModelli();

	glClearColor(0, 1, 1, 1);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glEnable(GL_CULL_FACE);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glViewport(0, 0, w, h);

	while (!glfwWindowShouldClose(window)) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		r.update();
		stack.load_identity();
		stack.push();
		stack.mult(tb[0].matrix());

		Lview = glm::lookAt(r.sunlight_direction(), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, -1.f)) * inverse(tb[0].matrix());
		Lpv = Lproj * Lview;

		glUseProgram(shader_program.program);
		glUniformMatrix4fv(shader_program["model_matrix"], 1, GL_FALSE, &stack.m()[0][0]);
		glUniformMatrix4fv(shader_program["uLightMatrix"], 1, GL_FALSE, &Lpv[0][0]);
		glUniform3fv(shader_program["spotDir"], 1, &(tb[0].matrix() * glm::vec4(0, -1, 0, 0))[0]);

		for (int i = 0; i < lampioni.size(); i++) {
			glUniform3fv(shader_program["sl[" + std::to_string(i) + "]"], 1, &(tb[0].matrix() * glm::translate(glm::vec3(0, lampioni[i].height, 0)) * glm::vec4(lampioni[i].pos, 1))[0]);
		}

		glUseProgram(depth_mapper.program);
		glUniformMatrix4fv(depth_mapper["model_matrix"], 1, GL_FALSE, &stack.m()[0][0]);
		glUniformMatrix4fv(depth_mapper["uLightMatrix"], 1, GL_FALSE, &Lpv[0][0]);

		glBindFramebuffer(GL_FRAMEBUFFER, fbo.id_fbo);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		glViewport(0, 0, smw, smh);
		draw_scene(depth_mapper);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, w, h);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, fbo.id_tex);
		draw_scene(shader_program);

		stack.pop();

		glViewport(0, 0, 300, 300);
		glDisable(GL_DEPTH_TEST);
		draw_texture(fbo.id_tex);
		glViewport(0, 0, w, h);
		glEnable(GL_DEPTH_TEST);

		check_gl_errors(__LINE__, __FILE__);

		glfwSwapBuffers(window);

		glfwPollEvents();
	}
}