#include "settings.h"
#include "test.h"
#include "imgui/imgui.h"

#include "tools.h"

#include <string>
#include <cmath>  

#define defaultAscendingAngle     0.470322600181172      //26°56'51"  = 0.470322600181172
#define GALLERY_FLOOR_WIDTH       2*cubit //((1.73f+46.12f)/cos(defaultAscendingAngle)) //
#define GALLERY_STEP_WIDTH        cubit / 7

// http://thegreatpyramidofgiza.ca/@Giza$Grand%20Gallery$Chapter_files/image003.jpg
float mul1 = 4.22f / 166.2f;

float galleryWallsVertical[] = {
	89.9f * mul1,
	129.9f * mul1,
	166.2f * mul1,
	211.7f * mul1,
	245.4f * mul1,
	278.7f * mul1,
	312.4f * mul1,
	8.74f
};


class Gallery : public Test
{
public:
	Gallery()
	{
		galleryBody = m_world->CreateBody(&bd);
		buildGallery(galleryBody);
	}
	void UpdateUI() override
	{
		ImGui::SetNextWindowPos(ImVec2(10.0f, 10.0f));
		ImGui::SetNextWindowSize(ImVec2(700.0f, 400.0f));
		ImGui::Begin("Controls", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

		//ImGui::ShowDemoWindow();

		ImGui::End();
	}
	
	float goldenAngle(b2Vec2 p1,b2Vec2 p2) {
		float p = p2.x - p1.x;
		float x1 = p1.y * p / (p1.y + p2.y);
		return atan2(-p1.y,x1);
	}

	void Step(Settings& settings) override
	{
		if (needToReset) {
			m_world->DestroyBody(galleryBody);

			galleryBody = m_world->CreateBody(&bd);
			buildGallery(galleryBody);

			needToReset = false;
		}

		for (int i = 0;i < sizeof(p) / sizeof(p[0]);i++) {
			char name[16];
			snprintf(name, sizeof(name), "%d", i);
			drawPoint(p[i], name);
		}


		// ---------------
		g_debugDraw.DrawSegment(p[0], p[0] + b2Vec2(0, galleryWallsVertical[7]), b2Color(0.5f, 0.5f, 0.5f));

		for (int i = 0;i < 8;i++) {
			b2Vec2 p1 = p[5 + i * 2];
			b2Vec2 p2 = p[6 + i * 2];

			b2Vec2 p3 = b2Vec2(p1.x, p[3].y);
			b2Vec2 p4 = b2Vec2(p2.x, p[3].y);

			g_debugDraw.DrawSegment(p1, p3, b2Color(0.5f, 0.5f, 0.5f));
			g_debugDraw.DrawSegment(p2, p4, b2Color(0.5f, 0.5f, 0.5f));

			//g_debugDraw.DrawSegment(p1, p[0], b2Color(0.8f, 0, 0));
			//g_debugDraw.DrawSegment(p2, p[0], b2Color(0.8f, 0, 0));
		
			/*
			Ray ray = Ray(p[5], 100, goldenAngle(p[5], p2), 1);
			drawRay(m_world, ray, b2Color(0, 0, 0.8f));

			ray = Ray(p[7], 100, goldenAngle(p[7], p2), 1);
			drawRay(m_world, ray, b2Color(0, 0.8f, 0));


			ray = Ray(p[9], 100, goldenAngle(p[9], p2), 1);
			drawRay(m_world, ray, b2Color(0, 0.8f, 0.8f));

			ray = Ray(p[11], 100, goldenAngle(p[11], p2), 1);
			drawRay(m_world, ray, b2Color(0.8f, 0, 0));
			
			ray = Ray(p[13], 100, goldenAngle(p[13], p2), 1);
			drawRay(m_world, ray, b2Color(0.8f, 0, 0.8f));

			ray = Ray(p[15], 100, goldenAngle(p[15], p2), 1);
			drawRay(m_world, ray, b2Color(0.8f, 0.8f, 0));

			ray = Ray(p[17], 100, goldenAngle(p[17], p2), 1);
			drawRay(m_world, ray, b2Color(0.8f, 0.8f, 0.8f));
			*/

			Ray ray = Ray(p[5 + i * 2], 100, goldenAngle(p[5 + i * 2], p[6 + i * 2]), 1);
			float d = drawRay(m_world, ray, b2Color(0.0, 0.8f, 0));

			char name[16];
			snprintf(name, sizeof(name), "%f", d);
			g_debugDraw.DrawString(b2Vec2(0, p[5 + i * 2].y), name);
		}



		Test::Step(settings);
	}

	static Test* Create()
	{
		return new Gallery;
	}

private:

	b2BodyDef bd;
	b2Body* galleryBody;

	bool needToReset = false;

	b2Vec2 p[22];

	void buildGallery(b2Body* body) {

		p[0] = b2Vec2(0,0);


		drawLine(body, p[0] + b2Vec2(-GALLERY_FLOOR_WIDTH / 2, 0), p[0] + b2Vec2(GALLERY_FLOOR_WIDTH / 2, 0));

		
		drawAbsorbLine(body, p[0] + b2Vec2(-GALLERY_FLOOR_WIDTH / 2, 0), p[0] + b2Vec2(-GALLERY_FLOOR_WIDTH / 2, cubit));
		drawAbsorbLine(body, p[0] + b2Vec2(GALLERY_FLOOR_WIDTH / 2, 0), p[0] + b2Vec2(GALLERY_FLOOR_WIDTH / 2, cubit));

		p[1] = p[0] + b2Vec2(-GALLERY_FLOOR_WIDTH / 2 - cubit, 0);
		p[2] = p[0] + b2Vec2(GALLERY_FLOOR_WIDTH / 2 + cubit, 0);

		p[3] = drawAbsorbLine(body, p[0] + b2Vec2(-GALLERY_FLOOR_WIDTH / 2, cubit), p[0] + b2Vec2(-GALLERY_FLOOR_WIDTH / 2 - cubit, cubit));
		p[4] = drawAbsorbLine(body, p[0] + b2Vec2(GALLERY_FLOOR_WIDTH / 2, cubit), p[0] + b2Vec2(GALLERY_FLOOR_WIDTH / 2+cubit, cubit));
		

		b2Vec2 p0_1 = p[3];
		b2Vec2 p0_2 = p[4];

		for (int i = 0;i < 8;i++) {
			
			b2Vec2 p1 = p[1] + b2Vec2(i * GALLERY_STEP_WIDTH, galleryWallsVertical[i]);
			b2Vec2 p2 = p[2] + b2Vec2(-i * GALLERY_STEP_WIDTH, galleryWallsVertical[i]);

			p[5 + i * 2] = p1;
			p[6 + i * 2] = p2;
			
			drawLine(body, p0_1, p1);
			drawLine(body, p0_2, p2);

			if (i < 7) {
				p0_1 = drawLine(body, p1, p1 + b2Vec2(GALLERY_STEP_WIDTH, 0));
				p0_2 = drawLine(body, p2, p2 + b2Vec2(-GALLERY_STEP_WIDTH, 0));
			}

		}
		drawLine(body, p[19], p[20]);

	}
	
};

static int testIndex = RegisterTest("Pyramid", "Gallery", Gallery::Create);
