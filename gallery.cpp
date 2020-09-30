#include "settings.h"
#include "test.h"
#include "imgui/imgui.h"

#include "tools.h"

#include <string>
#include <cmath>  

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
	
	void Step(Settings& settings) override
	{
		if (needToReset) {
			m_world->DestroyBody(galleryBody);

			galleryBody = m_world->CreateBody(&bd);
			buildGallery(galleryBody);

			needToReset = false;
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


	void buildGallery(b2Body* body) {

	}
	
};

static int testIndex = RegisterTest("Pyramid", "Gallery", Gallery::Create);
