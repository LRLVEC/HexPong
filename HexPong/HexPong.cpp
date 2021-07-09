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

	constexpr double scale = 0.9f;
	constexpr double playerW = 0.2f;
	constexpr double playerH = 0.05f;
	constexpr double frameRate = 144;
	constexpr double dt = 144 * 0.005 / frameRate;
	constexpr double leftLimit = playerW - 1;
	constexpr double rightLimit = 1 - playerW;
	constexpr double playerSpeed = 2.0;
	constexpr double ballSpeed = playerSpeed * 0.6 / rightLimit;

	enum Movement
	{
		Stop = 0,
		Left = 1,
		Right = 2,
	};
	struct Input
	{
		Movement move;
		double pos;

		Input()
			:
			move(Stop),
			pos(0)
		{
		}
		virtual double update(Movement _move)
		{
			switch (move = _move)
			{
			case Left:
				if (pos > leftLimit)
				{
					double tp(pos - playerSpeed * dt);
					pos = tp < leftLimit ? leftLimit : tp;
				}
				break;
			case Right:
				if (pos < rightLimit)
				{
					double tp(pos + playerSpeed * dt);
					pos = tp > rightLimit ? rightLimit : tp;
				}
				break;
			}
			return pos;
		}
	};


	struct Player
	{
		virtual Movement update()
		{

			return Stop;
		};
	};
	struct RealPlayer :Player
	{
		bool A_key;
		bool D_key;
		RealPlayer()
			:
			A_key(false),
			D_key(false)
		{
		}
		virtual Movement update() override
		{
			if (A_key ^ D_key)
			{
				if (A_key)return Left;
				else return  Right;
			}
			else return Stop;
		}
	};



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
					float h = scale * sqrt(3) / 2;
					float a = scale;
					float a2 = scale * 0.5;

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

			Buffer buffer;

			BufferConfig bufferArray;

			VertexAttrib positions;
			VertexAttrib colors;

			BorderRenderer(SourceManager* _sourceManager)
				:
				Program(_sourceManager, "Border", Vector<VertexAttrib*>{&positions, & colors}),
				borderLines(),
				buffer(&borderLines),
				bufferArray(&buffer, ArrayBuffer),
				positions(&bufferArray, 0, VertexAttrib::two,
					VertexAttrib::Float, false, sizeof(LineData::Vertex), 0, 0),
				colors(&bufferArray, 1, VertexAttrib::three,
					VertexAttrib::Float, false, sizeof(LineData::Vertex), sizeof(Math::vec2<float>), 0)
			{
				init();
			}
			void refreshBuffer()
			{

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
					double h = sqrt(3) / 2;
					rectangles[0].p[0] = { -playerW / 2, float(-h - playerH) };
					rectangles[0].p[1] = { playerW / 2, float(-h - playerH) };
					rectangles[0].p[2] = { playerW / 2, float(-h) };
					rectangles[0].p[3] = { -playerW / 2, float(-h) };
					for (unsigned int c0(0); c0 < 4; ++c0)
						rectangles[0].p[c0] *= scale;
					rectangles[0].p[4] = rectangles[0].p[0];
					rectangles[0].p[5] = rectangles[0].p[2];

					Math::mat2<double> rotation;
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
				void update(double* _offsets)
				{
					for (unsigned int c0(0); c0 < 6; ++c0)
					{
						double theta((Math::Pi * c0) / 3);
						double a2 = scale * 0.5 * _offsets[c0];
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
			void refreshBuffer(double* _offsets)
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

		RealPlayer realPlayer;
		Player defaultPlayer[5];
		Player* players[6];
		Input inputs[6];
		double offsets[6];

		Math::vec2<double> ball;
		Math::vec2<double> velocity;

		std::mt19937_64 mt;
		std::uniform_real_distribution<float> rd;
		std::uniform_real_distribution<float> rd1;

		HexPong()
			:
			sm(),
			renderer(&sm),
			playerRenderer(&sm),
			ballRenderer(&sm),
			realPlayer(),
			defaultPlayer{},
			players{ 0 },
			ball{ 0 },
			velocity{ 0,-ballSpeed },
			mt(time(nullptr)),
			rd(-1 + playerW, 1 - playerW),
			rd1(-0.5, 0.5)
		{
			players[0] = &realPlayer;
			for (unsigned int c0(0); c0 < 5; ++c0)
				players[c0 + 1] = defaultPlayer + c0;
		}

		bool isInHexagon()
		{
			using namespace Math;


			if (abs(0 - 2 * Pi) < 0.05)
				return true;//...
			else return false;
		}

		virtual void init(FrameScale const& _size) override
		{
			glViewport(0, 0, _size.w, _size.h);
			glPointSize(20);

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
				offsets[c0] = inputs[c0].update(players[c0]->update());

			ball += velocity * dt;

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
			case GLFW_MOUSE_BUTTON_LEFT: break;
			case GLFW_MOUSE_BUTTON_MIDDLE: break;
			case GLFW_MOUSE_BUTTON_RIGHT: break;
			}
		}
		virtual void mousePos(double _x, double _y) override {}
		virtual void mouseScroll(double _x, double _y) override {}
		virtual void key(GLFWwindow* _window, int _key, int _scancode, int _action, int _mods) override
		{
			switch (_key)
			{
			case GLFW_KEY_ESCAPE:
				if (_action == GLFW_PRESS)
					glfwSetWindowShouldClose(_window, true);
				break;
			case GLFW_KEY_A:realPlayer.A_key = _action; break;
			case GLFW_KEY_D:realPlayer.D_key = _action; break;
				//case GLFW_KEY_W: break;
				//case GLFW_KEY_S: break;
			}
		}
	};
}

int main()
{
	using namespace Math;

	vec2<float> a{ .5f, 1.f };
	vec2<float> b{ 1.f, .5f };

	printf("%f\n", acos((a, b) / (a.length() * b.length())));

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
	glfwSwapInterval(1);
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
