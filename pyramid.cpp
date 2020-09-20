// https://docs.google.com/presentation/d/1Vrz6EZI0J074KENVji01OVkGS4UIhxKx8XNgDmAPu3I/edit
// https://lah.ru/geometriya-velikoj-piramidy/

#include "settings.h"
#include "test.h"
#include "imgui/imgui.h"

#include <string>
#include <cmath>  

#define PI 3.14159265f
#define c 299792458                                   // Speed of light in vacuum
#define WIDTH (c / (PI*(sqrt(2)-1)))/1000000          // Pyramid width  = 230,380923883861
#define HEIGHT WIDTH * 14 / 22                        // Pyramid height = 146,606042471548

constexpr auto cubit = PI / 6;

#define SIDE sqrt(WIDTH*WIDTH+HEIGHT*HEIGHT)
#define VERTICAL_ANGLE atan(WIDTH/(2*HEIGHT))         // from top to side angle

#define KINGS_CHAMBER_LEVEL HEIGHT*(1-1/sqrt(2))

#define QUEEN_CHAMBER_ROOF_ANGLE      PI / 6
#define QUEEN_CHAMBER_HEIGHT          6.26f
#define QUEEN_CHAMBER_WIDTH           10 * cubit
#define QUEEN_CHAMBER_CENTER_LEVEL    (0.88f+0.83f)
#define QUEEN_CHAMBER_SMALLEST_BORDER 0.03f

#define KING_CHAMBER_HEIGHT           10 * cubit
#define KING_CHAMBER_WIDTH            10 * sqrt(5) * cubit / 2

constexpr auto shem = 6 * cubit / 5;

constexpr auto border0_bottom = 0.03f;
constexpr auto border0_top = 0.11f;

// entrance angle https://planetcalc.ru/71/
constexpr auto angle0 = 0.470322600181172;      //26°56'51"  = 0.470322600181172  

constexpr auto defaultAscendingAngle = 0.470322600181172;      //26°56'51"  = 0.470322600181172
constexpr auto defaultDescendingAngle = 0.46157171323714485;   //26°26'46"
constexpr auto defaultAngleRange = 5 * PI / 180;



b2Vec2 crossPoint(b2Vec2 p1, b2Vec2 p2, b2Vec2 p3, b2Vec2 p4) {
	return b2Vec2(
		((p1.x * p2.y - p1.y * p2.x) * (p3.x - p4.x) - (p1.x - p2.x) * (p3.x * p4.y - p3.y * p4.x)) / ((p1.x - p2.x) * (p3.y - p4.y) - (p1.y - p2.y) * (p3.x - p4.x)),
		((p1.x * p2.y - p1.y * p2.x) * (p3.y - p4.y) - (p1.y - p2.y) * (p3.x * p4.y - p3.y * p4.x)) / ((p1.x - p2.x) * (p3.y - p4.y) - (p1.y - p2.y) * (p3.x - p4.x))
	);
}

class RayCastClosestCallback : public b2RayCastCallback
{
public:
	RayCastClosestCallback()
	{
		m_hit = false;
	}

	float ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float fraction) override
	{
		b2Body* body = fixture->GetBody();
		void* userData = body->GetUserData();
		if (userData)
		{
			int32 index = *(int32*)userData;
			if (index == 0)
			{
				// By returning -1, we instruct the calling code to ignore this fixture and
				// continue the ray-cast to the next fixture.
				return -1.0f;
			}
		}

		m_hit = true;
		m_point = point;
		m_normal = normal;

		// By returning the current fraction, we instruct the calling code to clip the ray and
		// continue the ray-cast to the next fixture. WARNING: do not assume that fixtures
		// are reported in order. However, by clipping, we can always get the closest fixture.
		return fraction;
	}

	bool m_hit;
	b2Vec2 m_point;
	b2Vec2 m_normal;
};

class NextTo {
public:
	b2Vec2 v;
	NextTo(b2Vec2 _v) {
		v = b2Vec2(_v.x, _v.y);
	}
};

class Ray {
public:
	b2Vec2 from;
	float length;
	float angle;

	Ray(b2Vec2 _from, float _length, float _angle) {
		from = b2Vec2(_from.x, _from.y);
		length = _length;
		angle = _angle;
	}
};

class Piramid : public Test
{
public:
	Piramid()
	{
		pyramidBody = m_world->CreateBody(&bd);
		buildPyramid(pyramidBody);
	}

	void UpdateUI() override
	{
		ImGui::SetNextWindowPos(ImVec2(10.0f, 10.0f));
		ImGui::SetNextWindowSize(ImVec2(700.0f, 400.0f));
		ImGui::Begin("Controls", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

		//ImGui::ShowDemoWindow();

		if (ImGui::SliderAngle("Ascending Angle  ", &ascendingAngle, (defaultAscendingAngle - defaultAngleRange) * 180 / PI, (defaultAscendingAngle + defaultAngleRange) * 180 / PI, "%0f deg")) {
			needToReset = true;
		}

		if (ImGui::SliderAngle("Descending Angle", &descendingAngle, (defaultDescendingAngle - defaultAngleRange) * 180 / PI, (defaultDescendingAngle + defaultAngleRange) * 180 / PI, "%0f deg")) {
			needToReset = true;
		}

		if (ImGui::Checkbox("Enable Input Ray", &enableInputRay)) {
			needToReset = true;
		}

		ImGui::Separator();

		if (ImGui::Checkbox("Enable Queen Ray", &enableQueenRay)) {
			needToReset = true;
		}

		if (ImGui::SliderAngle("Queen Ray Angle", &queenAngle, 30, 180-30, "%0f deg")) {
			needToReset = true;
		}

		ImGui::End();
	}

	void Step(Settings& settings) override
	{
		if (needToReset) {
			m_world->DestroyBody(pyramidBody);

			pyramidBody = m_world->CreateBody(&bd);
			buildPyramid(pyramidBody);

			needToReset = false;
		}

		if (enableInputRay) {
			drawRainbowRay(
				p[14] + border0_top * b2Vec2(sin(descendingAngle), -cos(descendingAngle)),
				p[13] + border0_bottom * b2Vec2(-sin(descendingAngle), cos(descendingAngle)),
				angle0 + PI,
				50
			);
		}

		if (enableQueenRay) {
			drawRainbowRay(
				p[39],
				p[42],
				queenAngle,
				50
			);
		}

		for (int i = 0;i < sizeof(p) / sizeof(p[0]);i++) {
			char name[16];
			snprintf(name,sizeof(name),"%d",i);
			drawPoint(p[i], name);
		}

		g_debugDraw.DrawSegment(p[1], p[3], b2Color(0.5f, 0.5f, 0.5f));
		g_debugDraw.DrawSegment(p[3], p[4], b2Color(0.5f, 0.5f, 0.5f));
		g_debugDraw.DrawSegment(p[0], p[3], b2Color(0.5f, 0.5f, 0.5f));
		g_debugDraw.DrawSegment(p[0], p[4], b2Color(0.5f, 0.5f, 0.5f));
		g_debugDraw.DrawSegment(p[1], p[4], b2Color(0.5f, 0.5f, 0.5f));
		g_debugDraw.DrawSegment(p[5], p[0], b2Color(0.5f, 0.5f, 0.5f));
		g_debugDraw.DrawSegment(p[5], p[4], b2Color(0.5f, 0.5f, 0.5f));
		g_debugDraw.DrawSegment(p[5], p[6], b2Color(0.5f, 0.5f, 0.5f));
		g_debugDraw.DrawSegment(p[4], p[7], b2Color(0.5f, 0.5f, 0.5f));

		g_debugDraw.DrawSegment(p[0] + b2Vec2(-WIDTH / 2, KINGS_CHAMBER_LEVEL), p[6], b2Color(0.5f, 0.5f, 0.5f));

		// pyramid
		g_debugDraw.DrawSegment(p[0] + b2Vec2(-WIDTH / 2, 0), p[0] + b2Vec2(WIDTH / 2, 0), b2Color(1, 1, 1));           // ground base
		g_debugDraw.DrawSegment(p[0] + b2Vec2(0, HEIGHT), p[0] + b2Vec2(0, 0), b2Color(0.5f, 0.5f, 0.5f));  // vertical
		g_debugDraw.DrawSegment(p[0] + b2Vec2(-WIDTH / 2, 0), p[0] + b2Vec2(0, HEIGHT), b2Color(1, 1, 1));           // left side
		g_debugDraw.DrawSegment(p[0] + b2Vec2(WIDTH / 2, 0), p[0] + b2Vec2(0, HEIGHT), b2Color(1, 1, 1));           // right side
			   
		// cross
		g_debugDraw.DrawSegment(p[15], p[8], b2Color(0, 0, 0.5f));
		g_debugDraw.DrawSegment(p[8], p[17], b2Color(0, 0, 0.5f));
		g_debugDraw.DrawSegment(p[8], p[19], b2Color(0, 0, 0.5f));
		g_debugDraw.DrawSegment(p[15], p[19], b2Color(0, 0, 0.5f));
		g_debugDraw.DrawSegment(p[10], p[19], b2Color(0, 0, 0.5f));
		g_debugDraw.DrawSegment(p[10], p[17], b2Color(0, 0, 0.5f));

		// external coner
		g_debugDraw.DrawSegment(p[11], p[20], b2Color(0, 0, 0.5f));
		g_debugDraw.DrawSegment(p[20], p[12], b2Color(0, 0, 0.5f));

		// lower chamber
		g_debugDraw.DrawSegment(p[13], p[14], b2Color(0, 0, 0.5f));


		// gallery
		g_debugDraw.DrawSegment(p[16], p[24], b2Color(0, 0, 0.5f));
		g_debugDraw.DrawSegment(p[26], p[26]+b2Vec2(0,-1.17f), b2Color(0, 0, 0.5f));
		g_debugDraw.DrawSegment(p[24], p[48], b2Color(0, 0, 0.5f));

		// queen chamber center
		g_debugDraw.DrawSegment(p[30], p[26], b2Color(0, 0, 0.5f));
		g_debugDraw.DrawSegment(p[30], p[23], b2Color(0, 0, 0.5f));
		g_debugDraw.DrawSegment(p[31], p[32], b2Color(0.5f, 0.5f, 0.5f));
		g_debugDraw.DrawSegment(p[31], p[30], b2Color(0, 0, 0.5f));
		g_debugDraw.DrawSegment(p[31], p[31] + 5.09f * b2Vec2(-cos(QUEEN_CHAMBER_ROOF_ANGLE), -sin(QUEEN_CHAMBER_ROOF_ANGLE)), b2Color(0.5f, 0.5f, 0.5f));
		g_debugDraw.DrawSegment(p[31], p[31] + 5.09f * b2Vec2(cos(QUEEN_CHAMBER_ROOF_ANGLE), -sin(QUEEN_CHAMBER_ROOF_ANGLE)), b2Color(0.5f, 0.5f, 0.5f));

		g_debugDraw.DrawSegment(p[31], p[38], b2Color(0, 0, 0.5f));
		g_debugDraw.DrawSegment(p[35], p[39], b2Color(0, 0, 0.5f));

		g_debugDraw.DrawSegment(p[33]+ b2Vec2(0, 21.71f - 21.19f), p[32]+ b2Vec2(41.31f-33.2f, 21.71f - 21.19f), b2Color(0, 0, 0.5f));

		g_debugDraw.DrawSegment(p[42], p[40], b2Color(0, 0, 0.5f));


		// gallery left wall
		{
			g_debugDraw.DrawSegment(p[51], p[52], b2Color(0, 0, 0.5f));
			g_debugDraw.DrawSegment(p[71], p[77], b2Color(0, 0, 0.5f));
			g_debugDraw.DrawSegment(p[72], p[78], b2Color(0, 0, 0.5f));
			g_debugDraw.DrawSegment(p[73], p[79], b2Color(0, 0, 0.5f));
			g_debugDraw.DrawSegment(p[74], p[80], b2Color(0, 0, 0.5f));
			g_debugDraw.DrawSegment(p[75], p[81], b2Color(0, 0, 0.5f));
			g_debugDraw.DrawSegment(p[76], p[82], b2Color(0, 0, 0.5f));
		}
		// gallery right wall
		{
			g_debugDraw.DrawSegment(p[21], p[55], b2Color(0, 0, 0.5f));
			g_debugDraw.DrawSegment(p[63], p[56], b2Color(0, 0, 0.5f));
			g_debugDraw.DrawSegment(p[64], p[57], b2Color(0, 0, 0.5f));
			g_debugDraw.DrawSegment(p[65], p[58], b2Color(0, 0, 0.5f));
			g_debugDraw.DrawSegment(p[66], p[59], b2Color(0, 0, 0.5f));
			g_debugDraw.DrawSegment(p[67], p[60], b2Color(0, 0, 0.5f));
			g_debugDraw.DrawSegment(p[54], p[61], b2Color(0, 0, 0.5f));
		}

		// gallery horizontal levels
		{
			g_debugDraw.DrawSegment(p[77], p[62], b2Color(0, 0, 0.5f));
			g_debugDraw.DrawSegment(p[78], p[63], b2Color(0, 0, 0.5f));
			g_debugDraw.DrawSegment(p[79], p[64], b2Color(0, 0, 0.5f));
			g_debugDraw.DrawSegment(p[80], p[65], b2Color(0, 0, 0.5f));
			g_debugDraw.DrawSegment(p[81], p[66], b2Color(0, 0, 0.5f));
			g_debugDraw.DrawSegment(p[82], p[67], b2Color(0, 0, 0.5f));
			g_debugDraw.DrawSegment(p[83], p[68], b2Color(0, 0, 0.5f));

		}

		Test::Step(settings);
	}

	static Test* Create()
	{
		return new Piramid;
	}

private:

	float ascendingAngle = defaultAscendingAngle;
	float descendingAngle = defaultDescendingAngle;
	bool enableInputRay = false;
	
	bool enableQueenRay = false;
	float queenAngle = PI/6;


	b2BodyDef bd;
	b2Body* pyramidBody;

	bool needToReset = false;

	b2Vec2 p[84];

	void buildPyramid(b2Body* body) {
		
		// main points
		p[0] = b2Vec2(0, 0);
		p[1] = p[0] + b2Vec2(0, HEIGHT);
		p[2] = p[0] + b2Vec2(WIDTH / 2, 0);
		p[3] = p[0] + HEIGHT * (b2Vec2(0, 1) + b2Vec2(sin(2 * VERTICAL_ANGLE), -cos(2 * VERTICAL_ANGLE)));
		p[4] = p[3] + b2Vec2(0, -HEIGHT);
		p[5] = b2Vec2(0, p[4].y);
		p[6] = p[0] + b2Vec2(p[3].x, KINGS_CHAMBER_LEVEL);
		p[7] = p[0] + b2Vec2(0, KINGS_CHAMBER_LEVEL);
		p[8] = b2Vec2((p[6].x + p[5].x) / 2, (p[6].y + p[5].y) / 2);
		p[9] = crossPoint(p[1], p[2], p[5], p[6]);
		p[11] = crossPoint(p[1], p[2], p[8], p[8] + 100 * b2Vec2(cos(descendingAngle), sin(descendingAngle)));
		p[13] = p[8] + b2Vec2(-77.13f * cos(descendingAngle), -77.13f * sin(descendingAngle));
		p[14] = p[13] + 1.2f * b2Vec2(-sin(descendingAngle), cos(descendingAngle));
		p[16] = p[8] + b2Vec2(-39.28f  * cos(ascendingAngle), 39.28f * sin(ascendingAngle)),
		p[21] = p[16] + b2Vec2(-0.61f * cos(ascendingAngle), 0.61f * sin(ascendingAngle)) + 1.2f * b2Vec2(sin(ascendingAngle), cos(ascendingAngle));
		p[19] = p[8] + 1.2f * b2Vec2(-sin(descendingAngle), cos(descendingAngle));
		p[15] = crossPoint(p[8], p[16], p[14], p[19]);
		p[17] = p[8] + 1.2f * b2Vec2(sin(ascendingAngle), cos(ascendingAngle));
		p[18] = p[16] + 1.2f * b2Vec2(sin(ascendingAngle), cos(ascendingAngle));
		p[20] = p[11] + 1.2f * b2Vec2(-sin(descendingAngle), cos(descendingAngle));
		p[10] = crossPoint(p[19], p[20], p[17], p[18]);
		p[12] = crossPoint(p[19], p[20], p[1], p[2]);
		p[23] = p[16] + 46.12f * b2Vec2(-cos(ascendingAngle), sin(ascendingAngle));
		p[24] = p[23] + 1.73f * b2Vec2(-cos(ascendingAngle), sin(ascendingAngle));
		p[25] = p[16] + b2Vec2(-0.61f * cos(ascendingAngle), 0.61f * sin(ascendingAngle)) + b2Vec2(-0.15f * tan(ascendingAngle), -0.15f);
		p[26] = p[25] + b2Vec2(-(3.85f+0.68f), 1.17f);
		p[30] = b2Vec2(p[23].x,p[26].y);

		p[31] = p[30] + b2Vec2(0, QUEEN_CHAMBER_HEIGHT - QUEEN_CHAMBER_CENTER_LEVEL);
		p[32] = p[30] + b2Vec2(0, -QUEEN_CHAMBER_CENTER_LEVEL);
		p[33] = p[32] - b2Vec2(QUEEN_CHAMBER_WIDTH / 2, 0);
		p[34] = p[32] + b2Vec2(QUEEN_CHAMBER_WIDTH / 2, 0);

		p[35] = crossPoint(p[33], p[33] + b2Vec2(0, 2 * QUEEN_CHAMBER_HEIGHT), p[31], p[31] + 5.09f * b2Vec2(-cos(QUEEN_CHAMBER_ROOF_ANGLE), -sin(QUEEN_CHAMBER_ROOF_ANGLE)));
		p[36] = crossPoint(p[34], p[34] + b2Vec2(0, 2 * QUEEN_CHAMBER_HEIGHT), p[31], p[31] + 5.09f * b2Vec2(cos(QUEEN_CHAMBER_ROOF_ANGLE), -sin(QUEEN_CHAMBER_ROOF_ANGLE)));
		

		p[37] = b2Vec2(p[30].x + QUEEN_CHAMBER_WIDTH / 2, p[26].y);
		p[38] = crossPoint(p[31], p[37], p[33], p[34]);
		p[39] = crossPoint(p[35], p[37], p[33], p[34]);
		p[40] = p[25]+b2Vec2(-(33.2f - (p[16].x - p[25].x)), 0);

		p[42] = p[40] + b2Vec2( -(p[40].y - p[32].y)*(p[39]-p[35]).x/(p[35]-p[33]).y,-(p[40].y - p[32].y));
		p[43] = p[34] + b2Vec2(-(41.16f - 38.70f), 0);
		p[44] = p[43] + b2Vec2(-1.57f, 0);

		p[47] = p[31] + b2Vec2(0, -QUEEN_CHAMBER_SMALLEST_BORDER);
		p[45] = crossPoint(p[35] + b2Vec2(0, -0.14f), p[35] + b2Vec2(-1, -0.14f), p[47], p[47] + 5.09f * b2Vec2(-cos(QUEEN_CHAMBER_ROOF_ANGLE), -sin(QUEEN_CHAMBER_ROOF_ANGLE)));
		p[46] = crossPoint(p[36] + b2Vec2(0, -0.14f), p[36] + b2Vec2(1, -0.14f), p[47], p[47] + 5.09f * b2Vec2(cos(QUEEN_CHAMBER_ROOF_ANGLE), -sin(QUEEN_CHAMBER_ROOF_ANGLE)));

		p[48] = p[24] + b2Vec2(0, (43.03f - 42.9f));
		p[49] = p[23] + b2Vec2(0, 0.9f);

		p[50] = p[48] + b2Vec2(0, 1.11f);

		
		p[51] = crossPoint(p[24],p[23], b2Vec2(p[50].x + 0.55f,p[50].y), b2Vec2(p[50].x + 0.55f,p[50].y-10));
		p[52] = p[51] + b2Vec2(0, 8.74f);


		// http://thegreatpyramidofgiza.ca/@Giza$Grand%20Gallery$Chapter_files/image003.jpg
		float mul1 = 4.22f / 166.2f;

		float galleryWallsVertical[] = {
			89.9f * mul1,
			129.9f * mul1,
			166.2f * mul1,
			211.7f * mul1,
			245.4f * mul1,
			278.7f * mul1,
			312.4f * mul1 };

		// right gallery wall
		{
			p[61] = crossPoint(p[24], p[16], b2Vec2(p[21].x - 0.5f, p[21].y), b2Vec2(p[21].x - 0.5f, p[25].y));
			p[54] = p[61] + b2Vec2(0, 8.48f);
			p[55] = crossPoint(p[21], b2Vec2(p[21].x, p[21].y - 10), p[24], p[16]);

			p[56] = p[55] + b2Vec2(-0.120f, 0.120f * tan(ascendingAngle));
			p[57] = p[56] + b2Vec2(-0.080f, 0.080f * tan(ascendingAngle));
			p[58] = p[57] + b2Vec2(-0.090f, 0.090f * tan(ascendingAngle));
			p[59] = p[58] + b2Vec2(-0.060f, 0.060f * tan(ascendingAngle));
			p[60] = p[59] + b2Vec2(-0.075f, 0.075f * tan(ascendingAngle));

			p[62] = p[55] + b2Vec2(0, galleryWallsVertical[0]);
			p[63] = p[56] + b2Vec2(0, galleryWallsVertical[1]);
			p[64] = p[57] + b2Vec2(0, galleryWallsVertical[2]);
			p[65] = p[58] + b2Vec2(0, galleryWallsVertical[3]);
			p[66] = p[59] + b2Vec2(0, galleryWallsVertical[4]);
			p[67] = p[60] + b2Vec2(0, galleryWallsVertical[5]);
			p[68] = p[61] + b2Vec2(0, galleryWallsVertical[6]);
		}
		
		// left gallery wall
		{
			p[71] = p[24] + b2Vec2(0.09f, -0.09f * tan(ascendingAngle));
			p[72] = p[71] + b2Vec2(0.08f, -0.08f * tan(ascendingAngle));
			p[73] = p[72] + b2Vec2(0.07f, -0.07f * tan(ascendingAngle));
			p[74] = p[73] + b2Vec2(0.08f, -0.08f * tan(ascendingAngle));
			p[75] = p[74] + b2Vec2(0.10f, -0.10f * tan(ascendingAngle));
			p[76] = p[75] + b2Vec2(0.06f, -0.06f * tan(ascendingAngle));

			p[77] = p[71] + b2Vec2(0, galleryWallsVertical[0]);
			p[78] = p[72] + b2Vec2(0, galleryWallsVertical[1]);
			p[79] = p[73] + b2Vec2(0, galleryWallsVertical[2]);
			p[80] = p[74] + b2Vec2(0, galleryWallsVertical[3]);
			p[81] = p[75] + b2Vec2(0, galleryWallsVertical[4]);
			p[82] = p[76] + b2Vec2(0, galleryWallsVertical[5]);
			p[83] = p[51] + b2Vec2(0, galleryWallsVertical[6]);
		}


		drawLine(body, p[8], p[11]);
		drawLine(body, p[10], p[12]);
		drawLine(body, p[8], p[13]);
		drawLine(body, p[14], p[15]);
		drawLine(body, p[15], p[16]);
		drawLine(body, p[18], p[10]);
		drawLine(body, p[21], p[18]);



		// Lower Chamber
		{
			NextTo LowerChamber_13[7]{
				b2Vec2(-border0_bottom * sin(descendingAngle),border0_bottom * cos(descendingAngle)),
				b2Vec2(-8.91 - 8.28,0),
				b2Vec2(0,2.19 + 0.91),
				b2Vec2(8.36f,0),
				b2Vec2(0,-2.19f),
				b2Vec2(8.78 - 7.39,0),
				b2Vec2(0, 0)
			};

			drawPath(body, p[13], LowerChamber_13);

			NextTo LowerChamber_14[3]{
				b2Vec2(border0_top * sin(angle0),-border0_top * cos(angle0)),
				b2Vec2(-(8.27f - 3.21f),0),
				b2Vec2(0, 0)
			};

			drawPath(body, p[14], LowerChamber_14);
		}



		// Queen chamber
		{
			NextTo QueenChamber_p16[3]{
				b2Vec2(-0.61f * cos(ascendingAngle),0.61f * sin(ascendingAngle)),
				b2Vec2(-0.15f * tan(ascendingAngle),-0.15f),
				b2Vec2(0,0)
			};
			
			drawPath(body, p[16], QueenChamber_p16);
			drawLine(body, p[25], p[40]);
			drawLine(body, p[40],p[40]+ b2Vec2(0, -(p[25].y - p[32].y)));

			drawLine(body, p[39], p[42]);

		}

		// Gallery
		{
			// Right gallery wall
			{
				drawLine(body, p[21], b2Vec2(p[21].x, p[63].y));
				drawLine(body, b2Vec2(p[21].x, p[63].y), p[63]);

				drawLine(body, p[63], b2Vec2(p[63].x, p[64].y));
				drawLine(body, b2Vec2(p[63].x, p[64].y), p[64]);

				drawLine(body, p[64], b2Vec2(p[64].x, p[65].y));
				drawLine(body, b2Vec2(p[64].x, p[65].y), p[65]);

				drawLine(body, p[65], b2Vec2(p[65].x, p[66].y));
				drawLine(body, b2Vec2(p[65].x, p[66].y), p[66]);

				drawLine(body, p[66], b2Vec2(p[66].x, p[67].y));
				drawLine(body, b2Vec2(p[66].x, p[67].y), p[67]);

				drawLine(body, p[67], b2Vec2(p[67].x, p[68].y));
				drawLine(body, b2Vec2(p[67].x, p[68].y), b2Vec2(p[61].x,p[68].y));
				
				drawLine(body, p[68], p[54]);
			}

			// Left gallery wall
			{
				drawLine(body, p[50], b2Vec2(p[50].x, p[77].y));
				drawLine(body, b2Vec2(p[50].x, p[77].y), p[77]);

				drawLine(body, p[77], b2Vec2(p[77].x, p[78].y));
				drawLine(body, b2Vec2(p[77].x, p[78].y), p[78]);

				drawLine(body, p[78], b2Vec2(p[78].x, p[79].y));
				drawLine(body, b2Vec2(p[78].x, p[79].y), p[79]);

				drawLine(body, p[79], b2Vec2(p[79].x, p[80].y));
				drawLine(body, b2Vec2(p[79].x, p[80].y), p[80]);

				drawLine(body, p[80], b2Vec2(p[80].x, p[81].y));
				drawLine(body, b2Vec2(p[80].x, p[81].y), p[81]);

				drawLine(body, p[81], b2Vec2(p[81].x, p[82].y));
				drawLine(body, b2Vec2(p[81].x, p[82].y), p[82]);

				drawLine(body, p[82], b2Vec2(p[82].x, p[83].y));
				drawLine(body, b2Vec2(p[82].x, p[83].y), p[83]);

				drawLine(body, p[83], p[52]);
			}

			// Gallery ceiling
			{
				drawLine(body, p[52], p[54]);
			}


			// Right Gallery floor
			{
				NextTo RightGalleryFloor_p26[3]{
					b2Vec2(0,0.93f),
					b2Vec2(-1.53f * cos(ascendingAngle),1.53f * sin(ascendingAngle)),
					b2Vec2(0,0)
				};
				p[28] = drawPath(body, p[26], RightGalleryFloor_p26);
				p[29] = crossPoint(p[28], p[28] + b2Vec2(0, 1), p[23], p[16]);

				drawLine(body, p[28], p[29]);
				drawLine(body, p[29],p[23]);
			}
			
			// Left Gallery floor
			{
				drawLine(body, p[48], p[49]);
				drawLine(body, p[23], p[49]);
			}
		}

		// Queen Chamber
		{
			drawLine(body, p[31], p[35]);
			drawLine(body, p[31], p[36]);

			drawLine(body, p[35], p[35] + b2Vec2(0, -QUEEN_CHAMBER_SMALLEST_BORDER));
			drawLine(body, p[36], p[36] + b2Vec2(0, -QUEEN_CHAMBER_SMALLEST_BORDER));
			drawLine(body, p[35] + b2Vec2(0, -0.14f), p[33]);
			drawLine(body, p[35] + b2Vec2(0, -0.14f), p[45]);
			drawLine(body, p[36] + b2Vec2(0, -0.14f), p[37]);
			drawLine(body, p[36] + b2Vec2(0, -0.14f), p[46]);
			drawLine(body, p[35] + b2Vec2(0, -QUEEN_CHAMBER_SMALLEST_BORDER), p[45]);
			drawLine(body, p[36] + b2Vec2(0, -QUEEN_CHAMBER_SMALLEST_BORDER), p[46]);
			drawLine(body, p[26], p[37]);

			//floor
			drawLine(body, p[34], p[43]);
			drawLine(body, p[33], p[44]);
			drawLine(body, p[34], p[38]);
		}

		// King Chamber
		{
			NextTo KingChamber_p48[10]{
				b2Vec2(-6.83f,0),

				/*
				b2Vec2(-1.64f,0),
				b2Vec2(0,0.01f),
				b2Vec2(-1.2f,0),
				b2Vec2(0,-0.01f),
				b2Vec2(-1.79f - 2.2f,0),
				b2Vec2(0,0.02f),
				*/

				b2Vec2(-KING_CHAMBER_WIDTH,0),
				b2Vec2(0, KING_CHAMBER_HEIGHT),
				b2Vec2(KING_CHAMBER_WIDTH,0),
				b2Vec2(0, -(KING_CHAMBER_HEIGHT - (p[50].y - p[48].y))),
				b2Vec2(1.79f + 0.77f,0),
				b2Vec2(0,3.77f - (p[50].y - p[48].y)),
				b2Vec2(2.96f,0),
				b2Vec2(0,-(3.77f - (p[50].y - p[48].y))),
//				b2Vec2(1.23f,0),
				b2Vec2(0,0)
			};

			NextTo KingChamberBlock[5]{
				b2Vec2(0,1.33f),
				b2Vec2(-0.39f,0),
				b2Vec2(0,-1.33f),
				b2Vec2(0.39f,0),
				b2Vec2(0,0)
			};

			drawLine(body, p[50], drawPath(body, p[48], KingChamber_p48));
			drawPath(body, p[50] + b2Vec2(-1.24f - 0.54f, 0), KingChamberBlock);

		}

	}

	b2Vec2 reflect(b2Vec2 vector, b2Vec2 normal) {
		float num2 = vector.x * normal.x + vector.y * normal.y;
		return b2Vec2(vector.x - 2.0f * num2 * normal.x, vector.y - 2.0f * num2 * normal.y);
	}

	void drawPoint(b2Vec2 point, char* name) {
		g_debugDraw.DrawCircle(point, 0.1f, b2Color(1, 1, 1));
		g_debugDraw.DrawString(point, name);
	}
	void drawRay(Ray ray, b2Color color) {

		// initial source is little bit different to start raycasting from corner

		b2Vec2 source = b2Vec2(ray.from.x + 0.01f * cos(ray.angle), ray.from.y + 0.01f * sin(ray.angle));


		b2Vec2 destination = b2Vec2(ray.from.x + ray.length * cos(ray.angle), ray.from.y + ray.length * sin(ray.angle));

		for (int i = 0;i < 300;i++) {
			RayCastClosestCallback callback = RayCastClosestCallback();

			if ((destination - source).Length() > 0) {

				m_world->RayCast(&callback, source, destination);

				if (!callback.m_hit) {
					//g_debugDraw.DrawSegment(source, destination, b2Color(0.8f, 0.8f, 0.8f));
					//g_debugDraw.DrawCircle(source, 0.5f, b2Color(1, 0, 0));
					//g_debugDraw.DrawSegment(source, destination, b2Color(1, 0, 0));
					//g_debugDraw.DrawString(source, "ray to infinity");
					break;
				}
				else {
					g_debugDraw.DrawSegment(source, callback.m_point, color);
					//g_debugDraw.DrawSegment(callback.m_point, callback.m_point + 0.5 * callback.m_normal, b2Color(1, 0, 0)); // normal

					char str[16];
					float a = atan2(callback.m_point.y - source.y, callback.m_point.x - source.x) * 180 / PI;
					snprintf(str, sizeof(str), "o=%.2f", a);
					//g_debugDraw.DrawString(source, str);

					destination = callback.m_point + ray.length * reflect(callback.m_point - source, callback.m_normal);

					b2Vec2 direction = destination - callback.m_point;
					source = callback.m_point + 0.0001f * b2Vec2(direction.x / direction.Length(), direction.y / direction.Length());

				}
			}
			else {
				//g_debugDraw.DrawCircle(source, 0.5f, b2Color(0, 1, 0));
				//g_debugDraw.DrawString(source, "source=destination");
				break;
			}
		}
	}
	void drawRainbowRay(b2Vec2 start, b2Vec2 end, float angle, int rays) {
		for (float i = 0;i < rays;i++) {
			b2Vec2 point = start + (i / (float)rays) * (end - start);

			b2Color color = b2Color(
				0.5 + cos(5 * i / rays * 2 * PI / 6) / 2,
				0.5 + cos(5 * i / rays * 2 * PI / 6 + 2 * PI / 3) / 2,
				0.5 + cos(5 * i / rays * 2 * PI / 6 + 4 * PI / 3) / 2
			);
			drawRay(Ray(point, 100, angle), color);
		}
	}
	b2Vec2 drawPath(b2Body* body, b2Vec2 startPoint, NextTo* path) {
		int i = 0;

		b2Vec2 current = b2Vec2(startPoint.x, startPoint.y);
		b2EdgeShape shape;

		b2FixtureDef fd;
		fd.shape = &shape;
		fd.density = 0.0f;
		fd.friction = 0.6f;

		do {
			float x = current.x + path[i].v.x;
			float y = current.y + path[i].v.y;

			shape.SetTwoSided(current, b2Vec2(x, y));
			body->CreateFixture(&fd);
			current = b2Vec2(x, y);

			i++;

		} while (!((path[i].v.x == 0) && (path[i].v.y == 0)));
		return current;
	};
	void drawLine(b2Body* body, b2Vec2 startPoint, b2Vec2 endPoint) {
		b2EdgeShape shape;
		b2FixtureDef fd;
		fd.shape = &shape;
		fd.density = 0.0f;
		fd.friction = 0.6f;
		shape.SetTwoSided(startPoint, endPoint);
		body->CreateFixture(&fd);
	};
};

static int testIndex = RegisterTest("Collision", "Piramid", Piramid::Create);