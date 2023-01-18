#include "axxegro/math/Vec2.hpp"
#include "axxegro/prim/lldr.hpp"
#include "axxegro/resources/Resource.hpp"
#include <axxegro/axxegro.hpp>
#include <iterator>
#include <format>

#include <cmath>

#include <iostream>

using namespace al::ColorLiterals;


class PerlinNoiseGenerator
{
public:
	PerlinNoiseGenerator(uint32_t seed = 0x12345678): seed(seed) {}

	float operator()(al::Vec2f pos, int octaves = 1)
	{
		octaves = std::max(1, octaves);
		float sum = 0, max = 0;
		for(int i=0; i<octaves; i++) {
			double coeff = 1.0 / (1 << i);
			max += coeff;
			sum += baseOctave(pos * (1<<i)) * coeff;
		}
		return sum / max;
	}

	float baseOctave(al::Vec2f pos)
	{
		// https://en.wikipedia.org/wiki/Perlin_noise
		al::Vec2f floorPos(std::floor(pos.x), std::floor(pos.y));
		auto interp = pos - floorPos;

		float n[4];
		for(int i=0; i<4; i++) {
			al::Vec2f ipos = floorPos + al::Vec2f(!!(i&1), !!(i&2));
			n[i] = randomGradient(ipos).dot(pos-ipos);
		}

		float ix0 = interpolate(n[0], n[1], interp.x);
		float ix1 = interpolate(n[2], n[3], interp.x);
		
		return std::clamp(interpolate(ix0, ix1, interp.y), -1.0f, 1.0f) * 0.5 + 0.5;
	}
private:
	uint32_t seed = 0x12345678;

	static float interpolate(float a, float b, float weight) 
	{
		weight = std::clamp(weight, 0.0f, 1.0f);
		return (b - a) * (3.0 - weight * 2.0) * weight * weight + a;
	}

	al::Vec2f randomGradient(al::Vec2<int> pos)
	{
		uint32_t a = pos.x * 89733 + pos.y * 2879327 + 0xB16B00B5;
		a = a<<13 | a>>19;
		a ^= seed;
		a *= 0xD398A93F;
		return al::Vec2f(std::cos(a), std::sin(a));
	}
};


struct Mesh 
{
	std::vector<al::Vertex> vertices;
	std::vector<int> indices;
};


struct Camera {
	al::Vec3f pos;
	al::Vec2d rot;
	constexpr static al::Vec3f Up {0,0,-1};

	al::Vec3f forward() 
	{
		return al::Vec3f(
			std::cos(rot.x) * std::cos(rot.y),
			std::cos(rot.x) * std::sin(rot.y),
			std::sin(-rot.x)
		).normalized();
	}
	
	void rotate(al::Vec2f delta) 
	{
		rot += delta;
		rot.x = std::clamp(rot.x, -1.5706, 1.5706);
		rot.y = std::fmod(rot.y, ALLEGRO_PI * 2.0);
	}

	al::Vec3f right() 
	{
		return forward().cross(Up).normalized();
	}

	al::Transform transform() 
	{
		return al::Transform::Camera(pos, pos+forward(), Up);
	}
};

struct Skybox {
	al::Bitmap texture;
	Mesh skyboxMesh;

	Skybox(al::Bitmap&& texture)
		: texture(std::move(texture))
	{
		skyboxMesh = createSkyboxMesh();
		scaleUV();
	}

	static Mesh createSkyboxMesh()
	{
		static constexpr al::Vec3f positions[] = {
			{-1, -1, -1}, {-1, 1, -1}, {1, 1, -1}, {1, -1, -1}, // low part, clockwise
			{-1, -1, 1},  {-1, 1, 1},  {1, 1, 1},  {1, -1, 1}, // high part, clockwise
		};
		static constexpr al::Vec2f wallUV[] = {
			{0, 2}, {1, 2}, {2, 2}, {3, 2}, {4, 2},
			{0, 1}, {1, 1}, {2, 1}, {3, 1}, {4, 1}
		};
		static constexpr al::Vec2f floorCeilingUV[] {
			{1, 3}, {1, 2}, {2, 2}, {2, 3},
			{1, 0}, {1, 1}, {2, 1}, {2, 0}
		};
		static constexpr int floorCeilingIndices[] = {
			0, 1, 2, 0, 2, 3,
			4, 5, 6, 4, 6, 7
		};
		static constexpr int wallIndices[] = {
			0, 1, 5, 1, 5, 6,
			1, 2, 6, 2, 6, 7,
			2, 3, 7, 3, 7, 8,
			3, 4, 8, 4, 8, 9
		};
		std::array<al::Vertex, std::size(wallUV)> wallVertices;
		std::array<al::Vertex, std::size(floorCeilingUV)> floorCeilingVertices;

		for(unsigned i=0; i<wallVertices.size(); i++)
			wallVertices[i] = {positions[(i%5)%4 + 4*(i/5)], wallUV[i]};
		for(unsigned i=0; i<floorCeilingVertices.size(); i++)
			floorCeilingVertices[i] = {positions[i], floorCeilingUV[i]};
		
		Mesh result;
		for(auto v: wallVertices) result.vertices.push_back(v);
		for(auto v: floorCeilingVertices) result.vertices.push_back(v);
		for(auto i: wallIndices) result.indices.push_back(i);
		for(auto i: floorCeilingIndices) result.indices.push_back(i + wallVertices.size());

		return result;
	}

	void scaleUV()
	{
		for(auto& vtx: skyboxMesh.vertices) {
			vtx.setUV(vtx.getUV().hadamard({1.0/4.0,1.0/3.0}).hadamard(al::Vec2f(texture.size())));
		}
	}

	void render() 
	{
		al::DrawIndexedPrim(skyboxMesh.vertices, skyboxMesh.indices, texture);
	}
};



Mesh CreateTerrain(int sx, int sy, float height)
{
	Mesh result;
	PerlinNoiseGenerator perlin;
	for(int y=0; y<sy; y++) {
		for(int x=0; x<sx; x++) {
			al::Vec2f pos(x, y);
			float vh = height*perlin(pos*0.01, 8);
			al::Vertex vtx({pos.x, pos.y, vh}, pos*15.0, al::Gray(vh/height));
			result.vertices.push_back(vtx);
			if(x < sx-1 && y < sy-1) {
				for(auto offset: {0, 1, sx, 1, sx+1, sx}) {
					result.indices.push_back(y*sx+x+offset);
				}
			}
		}
	}

	return result;
}


int main()
{
	std::set_terminate(al::Terminate);
	al::FullInit();

	al::Display display(1024, 768, 0, {}, {
		{ALLEGRO_DEPTH_SIZE, 32}, 
		{ALLEGRO_FLOAT_DEPTH, 1},
		{ALLEGRO_VSYNC, 1}
	});
	display.setTitle("LPG Engine Demo");

	Mesh terrain = CreateTerrain(128, 128, 96);
	al::VertexBuffer terrainVB(terrain.vertices);
	al::IndexBuffer terrainIB(terrain.indices);

	Camera camera;
	camera.pos = {30, 30, 100};

	al::Transform proj = al::Transform::PerspectiveFOV(78, 0.01, 10000);
	al::EventLoop loop = al::EventLoop::Basic();
	
	auto builtinFont = al::Font::BuiltinFont();

	al::Bitmap::SetNewBitmapFlags(ALLEGRO_MIN_LINEAR | ALLEGRO_MAG_LINEAR);
	al::Bitmap rockTexture("data/rock.jpg");

	Skybox skybox(al::Bitmap("data/nightsky.jpg"));

	al::CurrentDisplay.hideCursor();
	loop.enableEscToQuit();
	loop.loopBody = [&](){
		
		//reset mouse to screen center
		if((al::GetMousePos() - (display.size()/2.0)).length() > 15) {
			al::SetMousePos(display.size()/2.0);
		}

		//handle keyboard input
		float movDelta = loop.getLastTickTime() * 30.0f;
		auto keyb = al::GetKeyboardState();
		if(al::IsKeyDown(keyb, ALLEGRO_KEY_W)) camera.pos += camera.forward() * movDelta;
		if(al::IsKeyDown(keyb, ALLEGRO_KEY_S)) camera.pos -= camera.forward() * movDelta;
		if(al::IsKeyDown(keyb, ALLEGRO_KEY_A)) camera.pos -= camera.right() * movDelta;
		if(al::IsKeyDown(keyb, ALLEGRO_KEY_D)) camera.pos += camera.right() * movDelta;

		//clear the framebuffer (technically unnecessary since we have a skybox)
		al::TargetBitmap.clearToColor(al::Black);
		proj.useProjection();

		//render the skybox
		al_set_render_state(ALLEGRO_DEPTH_TEST, 0);
		al::Transform::Camera({0,0,0}, camera.forward(), {0,0,-1}).use();
		skybox.render();

		//render the terrain
		al_set_render_state(ALLEGRO_DEPTH_TEST, 1);
		al_clear_depth_buffer(1.0);
		camera.transform().use();

		//al::DrawIndexedPrim(terrain.vertices, terrain.indices);
		al::DrawIndexedBuffer(terrainVB, terrainIB, rockTexture);
		
		//render the HUD
		al_set_render_state(ALLEGRO_DEPTH_TEST, 0);
		al::TargetBitmap.resetTransform();
		al::TargetBitmap.resetProjection();
		builtinFont.draw(std::format("{} fps", loop.getFPS()), al::White, {15, 15});

		al::CurrentDisplay.flip();
	};

	//rotate the camera when we move the mouse
	loop.eventDispatcher.setEventTypeHandler(ALLEGRO_EVENT_MOUSE_AXES, [&](const ALLEGRO_EVENT& ev){
		al::Vec2f delta(ev.mouse.dx, ev.mouse.dy);
		camera.rotate((delta * 0.002).transposed());
	});

	loop.run();
}
