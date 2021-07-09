#include <cstdio>
#include <GL/_Window.h>
#include <_Time.h>
#include <random>

namespace OpenGL
{
	struct Color
	{
		float r, g, b;
	};
	constexpr Color red = { 1.f, 0.f, 0.f };
	constexpr Color orange = { 1.f, 127.f / 255.f, 0.f };
	constexpr Color yellow = { 1.f, 1.f, 0.f };
	constexpr Color green = { 0.f, 1.f, 0.f };
	constexpr Color cyan = { 0.f, 1.f, 1.f };
	constexpr Color blue = { 0.f, 0.f, 1.f };
	constexpr Color purple = { 127.f / 255.f, 0.f, 1.f };

	constexpr float scale = 0.9f;
	constexpr float playerW = 0.2f;
	constexpr float playerH = 0.05f;

	struct HexPong :OpenGL
	{
		struct BorderRenderer :Program
		{
			struct LineData :Buffer::Data
			{
				struct Vertex
				{
					Math::vec2<float> pos;
					Color color;
				};
				Vertex lines[6];

				LineData()
					:
					Data(StaticDraw)
				{
					float h = scale * sqrtf(3) / 2;
					float a = scale;
					float a2 = scale * 0.5f;

					lines[0].pos = { -a2, -h };//user
					lines[1].pos = { a2, -h };
					lines[2].pos = { a, 0 };
					lines[3].pos = { a2 , h };
					lines[4].pos = { -a2, h };
					lines[5].pos = { -a, 0 };

					lines[0].color = cyan;
					lines[1].color = blue;
					lines[2].color = purple;
					lines[3].color = orange;
					lines[4].color = yellow;
					lines[5].color = green;
				}
				virtual void* pointer()override
				{
					return (void*)lines;
				}
				virtual unsigned int size()override
				{
					return sizeof(lines);
				}
			};

			LineData borderLines;

			Transform trans;
			Buffer buffer;
			Buffer transformBuffer;

			BufferConfig bufferArray;
			BufferConfig transformUnifrom;

			VertexAttrib positions;
			VertexAttrib colors;

			BorderRenderer(SourceManager* _sourceManager)
				:
				Program(_sourceManager, "Border", Vector<VertexAttrib*>{&positions, & colors}),
				borderLines(),
				trans({ {60.0,0.1,100},{0.05,0.8,0.05},{0.03},500.0 }),
				buffer(&borderLines),
				transformBuffer(&trans.bufferData),
				bufferArray(&buffer, ArrayBuffer),
				transformUnifrom(&transformBuffer, UniformBuffer, 0),
				positions(&bufferArray, 0, VertexAttrib::two,
					VertexAttrib::Float, false, sizeof(LineData::Vertex), 0, 0),
				colors(&bufferArray, 1, VertexAttrib::three,
					VertexAttrib::Float, false, sizeof(LineData::Vertex), sizeof(Math::vec2<float>), 0)
			{
				init();
			}
			void refreshBuffer()
			{
				trans.operate();
				if (trans.updated)
				{
					transformUnifrom.refreshData();
					trans.updated = false;
				}
			}
			virtual void initBufferData()override
			{
			}
			virtual void run() override
			{
				glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
				glClear(GL_COLOR_BUFFER_BIT);
				glDrawArrays(GL_LINE_LOOP, 0, 6);
			}
			void resize(int _w, int _h)
			{
				trans.resize(_w, _h);
				glViewport(0, 0, _w, _h);
			}
		};

		struct PlayerRenderer :Program
		{
			struct RectangleData :Buffer::Data
			{
				struct Rectangle
				{
					Math::vec2<float> p[6];
				};
				Rectangle rectangles[6];

				RectangleData()
					:
					Data(StaticDraw)
				{
					float h = sqrtf(3) / 2;
					rectangles[0].p[0] = { -playerW / 2, -h - playerH };
					rectangles[0].p[1] = { playerW / 2, -h - playerH };
					rectangles[0].p[2] = { playerW / 2, -h };
					rectangles[0].p[3] = { -playerW / 2, -h };
					for (unsigned int c0(0); c0 < 4; ++c0)
						rectangles[0].p[c0] *= scale;
					rectangles[0].p[4] = rectangles[0].p[0];
					rectangles[0].p[5] = rectangles[0].p[2];

					Math::mat2<float> rotation;
					for (unsigned int c0(1); c0 < 6; ++c0)
					{
						double theta((Math::Pi * c0) / 3);
						rotation.array[0][0] = cos(theta);
						rotation.array[0][1] = -sin(theta);
						rotation.array[1][0] = sin(theta);
						rotation.array[1][1] = cos(theta);
						for (unsigned int c1(0); c1 < 4; ++c1)
							rectangles[c0].p[c1] = (rotation, rectangles[0].p[c1]);
						rectangles[c0].p[4] = rectangles[c0].p[0];
						rectangles[c0].p[5] = rectangles[c0].p[2];
					}
				}
				virtual void* pointer()override
				{
					return (void*)rectangles;
				}
				virtual unsigned int size()override
				{
					return sizeof(rectangles);
				}
			};

			struct OffsetData :Buffer::Data
			{
				Math::vec4<float> offsets[6];

				OffsetData()
					:
					Data(DynamicDraw),
					offsets{ 0 }
				{
				}
				void update(float* _offsets)
				{
					for (unsigned int c0(0); c0 < 6; ++c0)
					{
						double theta((Math::Pi * c0) / 3);
						float a2 = scale * 0.5f * _offsets[c0];
						offsets[c0].data[0] = a2 * cos(theta);
						offsets[c0].data[1] = a2 * sin(theta);
					}
				}
				virtual void* pointer()override
				{
					return (void*)offsets;
				}
				virtual unsigned int size()override
				{
					return sizeof(offsets);
				}
			};

			RectangleData playerTriangles;
			OffsetData playerOffset;

			Buffer rectangleBuffer;
			Buffer offsetBuffer;

			BufferConfig bufferArray;
			BufferConfig offsetUniform;

			VertexAttrib positions;

			PlayerRenderer(SourceManager* _sourceManager)
				:
				Program(_sourceManager, "Player", Vector<VertexAttrib*>{&positions}),
				playerTriangles(),
				playerOffset(),
				rectangleBuffer(&playerTriangles),
				offsetBuffer(&playerOffset),
				bufferArray(&rectangleBuffer, ArrayBuffer),
				offsetUniform(&offsetBuffer, UniformBuffer, 1),
				positions(&bufferArray, 0, VertexAttrib::two,
					VertexAttrib::Float, false, sizeof(Math::vec2<float>), 0, 0)
			{
				init();
			}
			void refreshBuffer(float* _offsets)
			{
				playerOffset.update(_offsets);
				offsetUniform.refreshData();
			}
			virtual void initBufferData()override
			{
			}
			virtual void run() override
			{
				glDrawArrays(GL_TRIANGLES, 0, 6 * 2 * 3);
			}
		};

		struct BallRenderer :Program
		{
			struct BallData :Buffer::Data
			{
				Math::vec2<float> position;
				BallData()
					:
					Data(DynamicDraw),
					position{ 0 }
				{
				}
				void update(Math::vec2<float>pos)
				{
					position = pos * scale;
				}
				virtual void* pointer()override
				{
					return (void*)position.data;
				}
				virtual unsigned int size()override
				{
					return sizeof(position);
				}
			};

			BallData ballPos;
			Buffer ballBuffer;
			BufferConfig bufferArray;
			VertexAttrib positions;

			BallRenderer(SourceManager* _SourceManager)
				:
				Program(_SourceManager, "Ball", Vector<VertexAttrib*>{&positions}),
				ballPos(),
				ballBuffer(&ballPos),
				bufferArray(&ballBuffer, ArrayBuffer),
				positions(&bufferArray, 0, VertexAttrib::two,
					VertexAttrib::Float, false, sizeof(Math::vec2<float>), 0, 0)
			{
				init();
			}
			void refreshBuffer(Math::vec2<float>pos)
			{
				ballPos.update(pos);
				bufferArray.refreshData();
			}
			virtual void initBufferData()override
			{
			}
			virtual void run() override
			{
				glDrawArrays(GL_POINTS, 0, 1);
			}
		};

		SourceManager sm;
		BorderRenderer renderer;
		PlayerRenderer playerRenderer;
		BallRenderer ballRenderer;
		float offsets[6];
		Math::vec2<float> ball;

		std::mt19937_64 mt;
		std::uniform_real_distribution<float> rd;
		std::uniform_real_distribution<float> rd1;

		HexPong()
			:
			sm(),
			renderer(&sm),
			playerRenderer(&sm),
			ballRenderer(&sm),
			offsets{ 0 },
			ball{ 0 },
			mt(time(nullptr)),
			rd(-1 + playerW, 1 - playerW),
			rd1(-0.5, 0.5)
		{
		}
		virtual void init(FrameScale const& _size) override
		{
			glViewport(0, 0, _size.w, _size.h);
			glPointSize(20);

			renderer.trans.init(_size);
			renderer.transformUnifrom.dataInit();
			renderer.bufferArray.dataInit();

			playerRenderer.bufferArray.dataInit();
			playerRenderer.offsetUniform.dataInit();

			ballRenderer.bufferArray.dataInit();
		}
		virtual void run() override
		{
			/*
			glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);*/
			for (unsigned int c0(0); c0 < 6; ++c0)
				offsets[c0] = rd(mt);

			ball.data[0] = rd1(mt);
			ball.data[1] = rd1(mt);

			renderer.use();
			renderer.refreshBuffer();
			renderer.run();

			playerRenderer.use();
			playerRenderer.refreshBuffer(offsets);
			playerRenderer.run();

			ballRenderer.use();
			ballRenderer.refreshBuffer(ball);
			ballRenderer.run();
		}
		virtual void frameSize(int _w, int _h) override
		{
			renderer.resize(_w, _h);
		}
		virtual void framePos(int, int) override {}
		virtual void frameFocus(int) override {}
		virtual void mouseButton(int _button, int _action, int _mods) override
		{
			switch (_button)
			{
			case GLFW_MOUSE_BUTTON_LEFT:renderer.trans.mouse.refreshButton(0, _action); break;
			case GLFW_MOUSE_BUTTON_MIDDLE:renderer.trans.mouse.refreshButton(1, _action); break;
			case GLFW_MOUSE_BUTTON_RIGHT:renderer.trans.mouse.refreshButton(2, _action); break;
			}
		}
		virtual void mousePos(double _x, double _y) override
		{
			renderer.trans.mouse.refreshPos(_x, _y);
		}
		virtual void mouseScroll(double _x, double _y) override
		{
			if (_y != 0.0)
				renderer.trans.scroll.refresh(_y);
		}
		virtual void key(GLFWwindow* _window, int _key, int _scancode, int _action, int _mods) override
		{
			switch (_key)
			{
			case GLFW_KEY_ESCAPE:
				if (_action == GLFW_PRESS)
					glfwSetWindowShouldClose(_window, true);
				break;
			case GLFW_KEY_A:renderer.trans.key.refresh(0, _action); break;
			case GLFW_KEY_D:renderer.trans.key.refresh(1, _action); break;
			case GLFW_KEY_W:renderer.trans.key.refresh(2, _action); break;
			case GLFW_KEY_S:renderer.trans.key.refresh(3, _action); break;
			}
		}
	};
}

int main()
{
	OpenGL::OpenGLInit init(4, 5);
	Window::Window::Data winParameters
	{
		"HexPong",
		{
			{800,800},
			true,false
		}
	};
	Window::WindowManager wm(winParameters);
	OpenGL::HexPong test;
	wm.init(0, &test);
	glfwSwapInterval(12);
	FPS fps;
	fps.refresh();
	while (!wm.close())
	{
		wm.pullEvents();
		wm.render();
		wm.swapBuffers();
		fps.refresh();
		::printf("\r%.2lf    ", fps.fps);
		//fps.printFPS(1);
	}
	return 0;
}
