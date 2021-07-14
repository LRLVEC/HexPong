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

	constexpr double windowSize = 800;
	constexpr double scale = 0.9;
	constexpr double playerW = 0.2;
	constexpr double playerH = 0.05;
	constexpr double playerWHalf = playerW / 2;
	constexpr double frameRate = 80;
	constexpr double dt = 144 * 0.005 / frameRate;
	constexpr double dt2 = dt * dt * 0.5;
	constexpr double G = 0.3;
	constexpr double r0 = 0.1;
	constexpr double r03 = r0 * r0 * r0;
	constexpr double leftLimit = playerW - 1;
	constexpr double rightLimit = 1 - playerW;
	constexpr double playerSpeed = 2.0;
	constexpr double ballSpeed = playerSpeed * 0.9 / rightLimit;

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
	struct Physics
	{
		struct LineSegment
		{
			using vec2 = Math::vec2<double>;
			struct Intersection
			{
				using vec2 = Math::vec2<double>;

				bool intersected;
				double t1, t2;
				vec2 point;
				Intersection()
					:
					intersected(false),
					t1(0),
					t2(0),
					point{ 0 }
				{
				}
			};

			vec2 A, B;

			LineSegment() = default;
			LineSegment(vec2 _A, vec2 _B)
				:
				A(_A),
				B(_B)
			{

			}
			Intersection intersect(LineSegment b)
			{
				Intersection r;
				//todo: pre-test
				double l1((B - A).length()), l2((b.B - b.A).length());
				vec2 k1((B - A) / l1), k2((b.B - b.A) / l2);
				vec2 d(A - b.A);
				double s(k2[0] * k1[1] - k1[0] * k2[1]);
				if (s == 0)
				{
					r.intersected = false;
					return r;
				}
				r.t1 = (d[0] * k2[1] - k2[0] * d[1]) / s;
				r.t2 = (d[0] * k1[1] - k1[0] * d[1]) / s;
				r.point = (A + k1 * r.t1 + b.A + k2 * r.t2) / 2;
				if (r.t1 < 0 || r.t1 > l1 || r.t2 < 0 || r.t2 > l2)
					r.intersected = false;
				else
					r.intersected = true;
				return r;
			}
		};
		LineSegment lines[6];
		Math::vec2<double> r;
		Math::vec2<double> v;
		double offsets[6];
		Input inputs[6];
		LineSegment::Intersection its[6];

		Physics()
			:
			r{ 0.2, 0 },
			v{ 0, -1.2 * ballSpeed },
			offsets{ 0 },
			its{}
		{
			double h = sqrt(3) / 2;

			Math::vec2<double> vertices[6];

			vertices[0] = { -0.5, -h };
			vertices[1] = { 0.5, -h };
			vertices[2] = { 1, 0 };
			vertices[3] = { 0.5, h };
			vertices[4] = { -0.5, h };
			vertices[5] = { -1, 0 };
			for (unsigned int c0(0); c0 < 6; ++c0)
			{
				lines[c0].A = vertices[c0];
				lines[c0].B = vertices[(c0 + 1) % 6];
			}
		}
		void update(Player** players)
		{
			using namespace Math;
			double rr(r.length());
			vec2<double> a;
			if (rr > r0)a = r * (-G / pow(rr, 3));
			else a = r * (2 * G / pow(rr, 3));
			vec2<double> r1 = r + v * dt + a * dt2;
			v += a * dt;

			bool flag(true);
			LineSegment dr(r, r1);
			for (unsigned int c0(0); c0 < 6; ++c0)
				its[c0] = dr.intersect(lines[c0]);
			for (unsigned int c0(0); c0 < 6; ++c0)
				offsets[c0] = inputs[c0].update(players[c0]->update());
			for (unsigned int c0(0); c0 < 6; ++c0)
			{
				if (its[c0].intersected)
				{
					double offset(its[c0].t2 - (offsets[c0] + 1) / 2);
					if (abs(offset) < playerWHalf)
					{
						double theta((Math::Pi * c0) / 3);
						vec2<double> tau{ cos(theta), sin(theta) };
						vec2<double> n{ -sin(theta), cos(theta) };

						double ita(offset / playerWHalf);
						ita = ita * ita / 2;
						vec2<double> v1(n);
						if (offset >= 0)v1 += ita * tau;
						else v1 -= ita * tau;

						v = v1.normalize();
						r = its[c0].point + v * (ballSpeed * dt - its[c0].t1);
						v *= ballSpeed;
						flag = false;
					}
					else
					{
						printf("Player %u lost!\n", c0);
					}
					break;
				}
			}
			if (flag)r = r1;
		}

	};

	struct RealPlayer0 :Player
	{
		bool A_key;
		bool D_key;
		RealPlayer0()
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
	struct RealPlayer1 :Player
	{
		bool L_key;
		bool R_key;
		RealPlayer1()
			:
			L_key(false),
			R_key(false)
		{
		}
		virtual Movement update() override
		{
			if (L_key ^ R_key)
			{
				if (L_key)return Left;
				else return  Right;
			}
			else return Stop;
		}
	};

	struct EasyAI :Player
	{
		Physics* physics;
		unsigned int id;
		EasyAI(Physics* _physics, unsigned int _id)
			:
			physics(_physics),
			id(_id)
		{

		}
		virtual Movement update()override
		{
			using namespace Math;
			double theta((Math::Pi * id) / 3);
			vec2<double> n{ -sin(theta), cos(theta) };
			Physics::LineSegment dr(physics->r, physics->r + n);
			double t2 = dr.intersect(physics->lines[id]).t2;

			if (t2 >= -0.1 && t2 <= 1.1)
			{
				double target(t2 * 2 - 1);
				if (target > physics->inputs[id].pos)return Right;
				else return Left;
			}
			else
			{
				if (physics->inputs[id].pos > 0)return Left;
				else return Right;
			}
		}
	};
	struct BrutalAI :Player
	{
		Physics* physics;
		unsigned int id;

		BrutalAI(Physics* _physics, unsigned int _id)
			:
			physics(_physics),
			id(_id)
		{

		}
		virtual Movement update()override
		{
			double t1(physics->its[id].t1);
			double t2(physics->its[id].t2);
			if (t2 >= -0.5 && t2 <= 1.5 && t1 > 0)
			{
				double target(t2 * 2 - 1);
				if (target > physics->inputs[id].pos)return Right;
				else return Left;
			}
			else
			{
				if (physics->inputs[id].pos > 0)return Left;
				else return Right;
			}
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
				glDrawArrays(GL_LINE_LOOP, 0, 6);
			}
			void resize(int _w, int _h)
			{
				//glViewport(0, 0, _w, _h);
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
				Math::vec4<unsigned int> inversed;

				OffsetData()
					:
					Data(DynamicDraw),
					offsets{ 0 },
					inversed{ 0 }
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
				void inverse(bool _inversed)
				{
					inversed.data[0] = _inversed;
				}
				virtual void* pointer()override
				{
					return (void*)offsets;
				}
				virtual unsigned int size()override
				{
					return sizeof(offsets) + sizeof(inversed);
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
				offsetUniform(&offsetBuffer, UniformBuffer, 0),
				positions(&bufferArray, 0, VertexAttrib::two,
					VertexAttrib::Float, false, sizeof(Math::vec2<float>), 0, 0)
			{
				init();
			}
			void update(double* _offsets)
			{
				playerOffset.update(_offsets);
			}
			void refreshBuffer(bool _inversed)
			{
				playerOffset.inverse(_inversed);
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
				glPointSize(10);
				glDrawArrays(GL_POINTS, 0, 1);
			}
		};
		struct CircleRenderer :Program
		{
			struct CircleData :Buffer::Data
			{
				Math::vec2<float> position;
				CircleData()
					:
					Data(DynamicDraw),
					position{ 0 }
				{
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

			CircleData circleData;
			Buffer ballBuffer;
			BufferConfig bufferArray;
			VertexAttrib positions;

			CircleRenderer(SourceManager* _SourceManager)
				:
				Program(_SourceManager, "Circle", Vector<VertexAttrib*>{&positions}),
				circleData(),
				ballBuffer(&circleData),
				bufferArray(&ballBuffer, ArrayBuffer),
				positions(&bufferArray, 0, VertexAttrib::two,
					VertexAttrib::Float, false, sizeof(Math::vec2<float>), 0, 0)
			{
				init();
			}
			virtual void initBufferData()override
			{
			}
			virtual void run() override
			{
				glPointSize(windowSize * scale * r0);
				glDrawArrays(GL_POINTS, 0, 1);
			}
		};

		SourceManager sm;
		BorderRenderer renderer;
		PlayerRenderer playerRenderer;
		BallRenderer ballRenderer;
		CircleRenderer circleRenderer;

		RealPlayer0 realPlayer0;
		RealPlayer1 realPlayer1;
		//EasyAI simpleAIs[2];
		BrutalAI brutalAIs[4];
		Player* players[6];

		Physics physics;

		HexPong()
			:
			sm(),
			renderer(&sm),
			playerRenderer(&sm),
			ballRenderer(&sm),
			circleRenderer(&sm),
			realPlayer0(),
			realPlayer1(),
			brutalAIs{ {&physics,1},{&physics,2},{&physics,4},{&physics,5} },
			//simpleAIs{ {&physics,2}, {&physics,4} },
			players{ 0 },
			physics()
		{
			players[0] = &realPlayer0;
			players[3] = &realPlayer1;
			players[1] = brutalAIs;
			players[2] = brutalAIs + 1;
			players[4] = brutalAIs + 2;
			players[5] = brutalAIs + 3;
		}

		virtual void init(FrameScale const& _size) override
		{

			renderer.bufferArray.dataInit();

			playerRenderer.bufferArray.dataInit();
			playerRenderer.offsetUniform.dataInit();

			ballRenderer.bufferArray.dataInit();

			circleRenderer.bufferArray.dataInit();

		}
		virtual void run() override
		{
			physics.update(players);

			glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
			glClear(GL_COLOR_BUFFER_BIT);

			glViewport(0, 0, windowSize, windowSize);
			playerRenderer.update(physics.offsets);
			playerRenderer.refreshBuffer(false);

			renderer.use();
			renderer.refreshBuffer();
			renderer.run();

			playerRenderer.use();
			playerRenderer.run();

			circleRenderer.use();
			circleRenderer.run();

			ballRenderer.use();
			ballRenderer.refreshBuffer(physics.r);
			ballRenderer.run();

			glViewport(windowSize, 0, windowSize, windowSize);
			playerRenderer.refreshBuffer(true);
			renderer.use();
			renderer.run();

			playerRenderer.use();
			playerRenderer.run();

			circleRenderer.use();
			circleRenderer.run();

			ballRenderer.use();
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
			case GLFW_KEY_A:realPlayer0.A_key = _action; break;
			case GLFW_KEY_D:realPlayer0.D_key = _action; break;
			case GLFW_KEY_LEFT:realPlayer1.L_key = _action; break;
			case GLFW_KEY_RIGHT:realPlayer1.R_key = _action; break;
				//case GLFW_KEY_W: break;
				//case GLFW_KEY_S: break;
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
			{2 * OpenGL::windowSize, OpenGL::windowSize},
			true, false
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
