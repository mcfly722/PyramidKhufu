// good collection         http://countdowntothemessiah.com/Great_Pyramid/Davidsons_Pyramid_Records/Plate_Index.html
// math                    http://thegreatpyramidofgiza.ca/book/TheGreatPyramidofGIZA.pdf
// measurements            https://www.ronaldbirdsall.com/gizeh/petrie/c7.html#36
// grand gallery           http://www.palarch.nl/wp-content/miatello_l_examining_the_grand_gallery_in_the_pyramd_of_khufu_and_its_features_pjaee_7_6_2010.pdf
// linear sizes ang angles https://www.researchgate.net/figure/Inner-construction-of-the-Cheops-Pyramid-as-seen-from-the-east-with-the-linear_fig4_279852699
// gallery niches          https://khufupyramid.dk/inside-dimensions/grand-gallery

// https://docs.google.com/presentation/d/1Vrz6EZI0J074KENVji01OVkGS4UIhxKx8XNgDmAPu3I/edit
// https://lah.ru/geometriya-velikoj-piramidy/

#include "settings.h"
#include "test.h"
#include "imgui/imgui.h"

#include "tools.h"

#include <cmath>  

#define WIDTH (c / (PI*(sqrt(2)-1)))/1000000                                 // Pyramid width  = 230,380923883861
#define HEIGHT WIDTH * 14 / 22                                               // Pyramid height = 146,606042471548

#define SIDE                               sqrt(WIDTH*WIDTH+HEIGHT*HEIGHT)   // pyramid side
#define VERTICAL_ANGLE                     atan(WIDTH/(2*HEIGHT))            // from top to side angle

#define KINGS_CHAMBER_LEVEL                HEIGHT*(1-1/sqrt(2))

#define QUEEN_CHAMBER_ROOF_ANGLE           PI / 6
#define QUEEN_CHAMBER_HEIGHT               6.26f
#define QUEEN_CHAMBER_WIDTH                10 * cubit
#define QUEEN_CHAMBER_CENTER_LEVEL         (0.88f+0.83f)
#define QUEEN_CHAMBER_SMALLEST_BORDER      0.03f

#define KING_CHAMBER_HEIGHT                10 * cubit
#define KING_CHAMBER_WIDTH                 10 * sqrt(5) * cubit / 2

#define GALLERY_CEILING_FIRST_STEP_WIDTH   0.37f

#define GALLERY_HOLE_SHORT_DEPTH           0.18f
#define GALLERY_HOLE_LONG_DEPTH            0.18f
#define GALLERY_HOLE_SHORT_WIDTH_MUL       1/6.526f      // coefficient
#define GALLERY_HOLE_LONG_WIDTH_MUL        1.13f/6.526f  // coefficient
#define GALLERY_HOLES_SPACE_MUL            2.198f/6.526f // coefficient

#define GALLERY_NICHE_HEIGH_MUL            1.15f         // coefficient
#define GALLERY_NICHE_WIDTH_MUL            0.53f         // coefficient



constexpr auto shem = 6 * cubit / 5;

constexpr auto border0_bottom = 0.03f;
constexpr auto border0_top = 0.11f;

// entrance angle https://planetcalc.ru/71/
constexpr auto angle0 = 0.470322600181172;      //26°56'51"  = 0.470322600181172  

constexpr auto defaultAscendingAngle = 0.470322600181172;      //26°56'51"  = 0.470322600181172
constexpr auto defaultDescendingAngle = 0.46157171323714485;   //26°26'46"
constexpr auto defaultAngleRange =  PI / 180;

b2Vec2 crossPoint(b2Vec2 p1, b2Vec2 p2, b2Vec2 p3, b2Vec2 p4) {
	return b2Vec2(
		((p1.x * p2.y - p1.y * p2.x) * (p3.x - p4.x) - (p1.x - p2.x) * (p3.x * p4.y - p3.y * p4.x)) / ((p1.x - p2.x) * (p3.y - p4.y) - (p1.y - p2.y) * (p3.x - p4.x)),
		((p1.x * p2.y - p1.y * p2.x) * (p3.y - p4.y) - (p1.y - p2.y) * (p3.x * p4.y - p3.y * p4.x)) / ((p1.x - p2.x) * (p3.y - p4.y) - (p1.y - p2.y) * (p3.x - p4.x))
	);
}


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

		if (ImGui::TreeNode("Corridors"))
		{
			if (ImGui::SliderAngle("Ascending Angle  ", &ascendingAngle, (defaultAscendingAngle - defaultAngleRange) * 180 / PI, (defaultAscendingAngle + defaultAngleRange) * 180 / PI, "%0f deg")) {
				needToReset = true;
				//angle2minutesAndSeconds
			}
			char str[30];
			angle2minutesAndSeconds(str, sizeof(str), "Ascending angle = ", ascendingAngle);
			ImGui::Text(str);

			if (ImGui::SliderAngle("Descending Angle", &descendingAngle, (defaultDescendingAngle - defaultAngleRange) * 180 / PI, (defaultDescendingAngle + defaultAngleRange) * 180 / PI, "%0f deg")) {
				needToReset = true;
			}
			angle2minutesAndSeconds(str, sizeof(str), "Descending angle = ", descendingAngle);
			ImGui::Text(str);


			if (ImGui::Checkbox("Show Corridors Problem", &showCorridorsCrossingProblem)) {
				needToReset = true;
			}
			

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Bottom Chamber"))
		{
			if (ImGui::Checkbox("Enable Input Ray", &enableInputRay)) {
				needToReset = true;
			}

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Queen Chamber"))
		{
			if (ImGui::Checkbox("Enable Ray", &enableQueenRay)) {
				needToReset = true;
			}

			if (ImGui::SliderAngle("Ray Angle", &queenAngle, 30, 180 - 30, "%0f deg")) {
				needToReset = true;
			}

			ImGui::TreePop();
		}

		if (ImGui::TreeNodeEx("Gallery"))
		{
			if (ImGui::Checkbox("Ceiling parallel to floor", &ceilingParallelToFloor)) {
				needToReset = true;
			}

			if (ImGui::SliderFloat("Ceiling Vertical Offset", &galleryCeilingOffset, 0.0f, 12.0f, "%.3f")) {
				needToReset = true;
			}

			// Gallery Left Wall Mode ratio group
			{
				if (ImGui::RadioButton("Left Wall levels is Horizontal", &leftGalleryWallMode,Horizontal)) {
					leftGalleryWallMode = Horizontal;
					needToReset = true;
				}
				ImGui::SameLine();
				if (ImGui::RadioButton("Left Wall levels is Parallel", &leftGalleryWallMode,Parallel)) {
					leftGalleryWallMode = Parallel;
					needToReset = true;
				}
			}

			{
				if (ImGui::RadioButton("Right Wall levels is Horizontal", &rightGalleryWallMode, Horizontal)) {
					rightGalleryWallMode = Horizontal;
					needToReset = true;
				}
				ImGui::SameLine();
				if (ImGui::RadioButton("Right Wall levels is Parallel", &rightGalleryWallMode, Parallel)) {
					rightGalleryWallMode = Parallel;
					needToReset = true;
				}
			}
			// Gallery beams
			{
				ImGui::Text("Beams material type:");
				ImGui::SameLine();
				if (ImGui::RadioButton("Absorb", &galleryBeamsMode, Absorb)) {
					galleryBeamsMode = Absorb;
					needToReset = true;
				}
				ImGui::SameLine();
				if (ImGui::RadioButton("Transparent", &galleryBeamsMode, Transparent)) {
					galleryBeamsMode = Transparent;
					needToReset = true;
				}
				ImGui::SameLine();
				if (ImGui::RadioButton("Reflect", &galleryBeamsMode, Reflect)) {
					galleryBeamsMode = Reflect;
					needToReset = true;
				}
			}


			ImGui::TreePop();
		}

		ImGui::End();
	}

	void drawNiche(b2Vec2 bottomCenter,float width, float hight) {
		b2Vec2 p1 = bottomCenter + b2Vec2(width / 2, -width * tan(ascendingAngle) / 2);
		b2Vec2 p2 = bottomCenter + b2Vec2(-width / 2, width * tan(ascendingAngle) / 2);
		b2Vec2 p3 = bottomCenter + b2Vec2(-width / 2, hight);
		b2Vec2 p4 = bottomCenter + b2Vec2(width / 2, hight);
		g_debugDraw.DrawSegment(p1, p2, b2Color(0, 0.5f, 0));
		g_debugDraw.DrawSegment(p2, p3, b2Color(0, 0.5f, 0));
		g_debugDraw.DrawSegment(p3, p4, b2Color(0, 0.5f, 0));
		g_debugDraw.DrawSegment(p4, p1, b2Color(0, 0.5f, 0));
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
				m_world,
				p[14] + border0_top * b2Vec2(sin(descendingAngle), -cos(descendingAngle)),
				p[13] + border0_bottom * b2Vec2(-sin(descendingAngle), cos(descendingAngle)),
				angle0 + PI,
				50
			);
		}

		if (enableQueenRay) {
			drawRainbowRay(
				m_world,
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
		g_debugDraw.DrawSegment(p[0] + b2Vec2(0, HEIGHT), p[0] + b2Vec2(0, 0), b2Color(0.5f, 0.5f, 0.5f));              // vertical
		g_debugDraw.DrawSegment(p[0] + b2Vec2(-WIDTH / 2, 0), p[0] + b2Vec2(0, HEIGHT), b2Color(1, 1, 1));              // left side
		g_debugDraw.DrawSegment(p[0] + b2Vec2(WIDTH / 2, 0), p[0] + b2Vec2(0, HEIGHT), b2Color(1, 1, 1));               // right side
			   
		// ascending,descending corridors cross
		{
			g_debugDraw.DrawSegment(p[15], p[8], b2Color(0, 0.5f, 0.5f));
			g_debugDraw.DrawSegment(p[8], p[17], b2Color(0, 0.5f, 0.5f));
			g_debugDraw.DrawSegment(p[8], p[19], b2Color(0, 0.5f, 0.5f));
			g_debugDraw.DrawSegment(p[15], p[19], b2Color(0, 0.5f, 0.5f));
			g_debugDraw.DrawSegment(p[10], p[19], b2Color(0, 0.5f, 0.5f));
			g_debugDraw.DrawSegment(p[10], p[17], b2Color(0, 0.5f, 0.5f));

			if (showCorridorsCrossingProblem) {
				g_debugDraw.DrawSegment(p[22],p[21], b2Color(1, 0, 0));
				g_debugDraw.DrawSegment(p[22], p[20], b2Color(1, 0, 0));
				g_debugDraw.DrawSegment(p[22], p[8], b2Color(1, 0, 0));
			}

		}


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
		g_debugDraw.DrawSegment(p[30], b2Vec2(p[30].x,p[88].y), b2Color(0, 0, 0.5f));
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

		// gallery ceiling
		{
			g_debugDraw.DrawSegment(p[52], p[84], b2Color(0, 0, 0.5f));
			g_debugDraw.DrawSegment(p[84], p[54], b2Color(0, 0, 0.5f));
			g_debugDraw.DrawSegment(p[54], p[85], b2Color(0, 0, 0.5f));
			g_debugDraw.DrawSegment(p[85], p[52], b2Color(0, 0, 0.5f));

			float numberOfSteps = 36;
			float stepWidth = (sqrt((p[52] - p[85]).LengthSquared()) - GALLERY_CEILING_FIRST_STEP_WIDTH) / numberOfSteps;
			//float numberOfSteps = ceil(((sqrt((p[52] - p[85]).LengthSquared()) - GALLERY_CEILING_FIRST_STEP_WIDTH) / 1.2f));
			float angle = atan2(p[52].y-p[85].y,p[85].x-p[52].x);
			float stepHight = sqrt((p[84] - p[52]).LengthSquared()) / numberOfSteps;
			


			for (int i = 0;i < numberOfSteps;i++) {
				b2Vec2 p1 = p[52] + (GALLERY_CEILING_FIRST_STEP_WIDTH + stepWidth * i) * b2Vec2(cos(angle), -sin(angle));
				b2Vec2 p2 = p[84] + (GALLERY_CEILING_FIRST_STEP_WIDTH + stepWidth * i) * b2Vec2(cos(angle), -sin(angle));
				g_debugDraw.DrawSegment(p1, p2, b2Color(0, 0, 0.5f));
				b2Vec2 p3 = p[52] + (stepHight * i) * b2Vec2(sin(angle), cos(angle));
				b2Vec2 p4 = p[85] + (stepHight * i) * b2Vec2(sin(angle), cos(angle));
				g_debugDraw.DrawSegment(p3, p4, b2Color(0, 0, 0.5f));
				
				b2Vec2 p5 = crossPoint(p1, p2, p3, p4);

				char name[16];
				snprintf(name, sizeof(name), "%d", i + 1);
				g_debugDraw.DrawString(p5+0.4f*b2Vec2(cos(angle),-sin(angle)), name);
			}
		}

		// gallery floor
		// https://khufupyramid.dk/inside-dimensions/grand-gallery
		{
			g_debugDraw.DrawSegment(p[90], p[91], b2Color(0, 0.5f, 0));

			float stepSize = 6.526f*sqrt((p[90] - p[91]).LengthSquared())/88.036f;
			
			for (int i = 0;i < 14;i++) {
				char name[16];

				// small hole
				b2Vec2 step_i = p[91] + stepSize * i * b2Vec2(-cos(ascendingAngle), sin(ascendingAngle));

				b2Vec2 p0 = step_i + cubit * b2Vec2(-sin(ascendingAngle), -cos(ascendingAngle));
				g_debugDraw.DrawSegment(step_i, p0, b2Color(0, 0.5f, 0));

				b2Vec2 p1 = step_i + GALLERY_HOLE_SHORT_DEPTH * b2Vec2(-sin(ascendingAngle), -cos(ascendingAngle));
				b2Vec2 p3 = step_i + (GALLERY_HOLE_SHORT_WIDTH_MUL * stepSize) * b2Vec2(-cos(ascendingAngle), sin(ascendingAngle));
				b2Vec2 p2 = p3 + GALLERY_HOLE_SHORT_DEPTH * b2Vec2(-sin(ascendingAngle), -cos(ascendingAngle));

				g_debugDraw.DrawSegment(step_i, p1, b2Color(0, 0.5f, 0.5f));
				g_debugDraw.DrawSegment(p1, p2, b2Color(0, 0.5f, 0.5f));
				g_debugDraw.DrawSegment(p2, p3, b2Color(0, 0.5f, 0.5f));
				g_debugDraw.DrawSegment(p3, step_i, b2Color(0, 0.5f, 0.5f));

				snprintf(name, sizeof(name), "%ds", i + 1);
				g_debugDraw.DrawString(b2Vec2((step_i.x + p2.x) / 2, (step_i.y + p2.y) / 2), name);   // Short Hole
				

				// long hole
				b2Vec2 p4 = p3 + GALLERY_HOLES_SPACE_MUL*stepSize * b2Vec2(-cos(ascendingAngle), sin(ascendingAngle));
				b2Vec2 p5 = p4 + GALLERY_HOLE_LONG_DEPTH * b2Vec2(-sin(ascendingAngle), -cos(ascendingAngle));
				b2Vec2 p7 = p4 + (GALLERY_HOLE_LONG_WIDTH_MUL * stepSize) * b2Vec2(-cos(ascendingAngle), sin(ascendingAngle));
				b2Vec2 p6 = p7 + GALLERY_HOLE_LONG_DEPTH * b2Vec2(-sin(ascendingAngle), -cos(ascendingAngle));

				g_debugDraw.DrawSegment(p4, p5, b2Color(0, 0.5f, 0.5f));
				g_debugDraw.DrawSegment(p5, p6, b2Color(0, 0.5f, 0.5f));
				g_debugDraw.DrawSegment(p6, p7, b2Color(0, 0.5f, 0.5f));
				g_debugDraw.DrawSegment(p7, p4, b2Color(0, 0.5f, 0.5f));

				snprintf(name, sizeof(name), "%dL", i + 1);
				g_debugDraw.DrawString(b2Vec2((p4.x + p6.x) / 2, (p4.y + p6.y) / 2), name);   // Long Hole

				if (i > 0) {
					// short cutting
					b2Vec2 p8 = step_i + GALLERY_HOLE_SHORT_WIDTH_MUL * stepSize * b2Vec2(0, 0.19f);
					b2Vec2 p9 = p8 + (GALLERY_HOLE_SHORT_WIDTH_MUL * stepSize) * b2Vec2(-cos(ascendingAngle), sin(ascendingAngle));
					b2Vec2 p10 = p9 + GALLERY_HOLE_SHORT_WIDTH_MUL * stepSize * b2Vec2(0, 0.38f);
					b2Vec2 p11 = p8 + GALLERY_HOLE_SHORT_WIDTH_MUL * stepSize * b2Vec2(0, 0.38f);
					g_debugDraw.DrawSegment(p8, p9, b2Color(0, 0.5f, 0.5f));
					g_debugDraw.DrawSegment(p9, p10, b2Color(0, 0.5f, 0.5f));
					g_debugDraw.DrawSegment(p10, p11, b2Color(0, 0.5f, 0.5f));
					g_debugDraw.DrawSegment(p11, p8, b2Color(0, 0.5f, 0.5f));

					if (i < 13) {
						// long cutting
						b2Vec2 p12 = p4 + GALLERY_HOLE_SHORT_WIDTH_MUL * stepSize * b2Vec2(0, 0.19f);
						b2Vec2 p13 = p12 + (GALLERY_HOLE_LONG_WIDTH_MUL * stepSize) * b2Vec2(-cos(ascendingAngle), sin(ascendingAngle));
						b2Vec2 p14 = p13 + GALLERY_HOLE_SHORT_WIDTH_MUL * stepSize * b2Vec2(0, 0.38f);
						b2Vec2 p15 = p12 + GALLERY_HOLE_SHORT_WIDTH_MUL * stepSize * b2Vec2(0, 0.38f);
						g_debugDraw.DrawSegment(p12, p13, b2Color(0, 0.5f, 0.5f));
						g_debugDraw.DrawSegment(p13, p14, b2Color(0, 0.5f, 0.5f));
						g_debugDraw.DrawSegment(p14, p15, b2Color(0, 0.5f, 0.5f));
						g_debugDraw.DrawSegment(p15, p12, b2Color(0, 0.5f, 0.5f));

						float w = GALLERY_NICHE_WIDTH_MUL * (GALLERY_HOLE_SHORT_WIDTH_MUL * stepSize);
						float h = GALLERY_NICHE_HEIGH_MUL * (GALLERY_HOLE_SHORT_WIDTH_MUL * stepSize);

						drawNiche(b2Vec2((p3.x + step_i.x) / 2, (p3.y + step_i.y) / 2), w, h);
						drawNiche(b2Vec2((p7.x + p4.x) / 2, (p7.y + p4.y) / 2), w, h);

						drawRay(m_world, Ray(p12, 100, PI / 2 - ascendingAngle, 0), b2Color(0, 0.5f, 0.5f));
					}

					drawRay(m_world, Ray(p8, 100, PI / 2 - ascendingAngle, 0), b2Color(0, 0.5f, 0.5f));

				}
			}

		}

		Test::Step(settings);
	}

	static Test* Create()
	{
		return new Piramid;
	}

private:
	enum GalleryWallsMode
	{
		Horizontal,
		Parallel
	};

	enum MaterialType
	{
		Transparent,
		Reflect,
		Absorb
	};

	// control parameters 
	float ascendingAngle = defaultAscendingAngle;
	float descendingAngle = defaultDescendingAngle;
	bool showCorridorsCrossingProblem = false;
	bool enableInputRay = false;

	bool enableQueenRay = false;
	float queenAngle = PI / 6;

	bool ceilingParallelToFloor = true;
	float galleryCeilingOffset = 1;
	int leftGalleryWallMode  = Horizontal;      // http://thepyramids.org/images/giza/231_026_great_pyramid.jpg
	int rightGalleryWallMode = Parallel;

	int galleryBeamsMode = Absorb;

	b2BodyDef bd;
	b2Body* pyramidBody;


	bool needToReset = false;

	b2Vec2 p[92];

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
		p[22] = p[8] + b2Vec2(0,1.2f/cos(descendingAngle));
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
			
			if (ceilingParallelToFloor) {
				p[54] = p[61] + p[52]-p[51];
			} else {
				p[54] = p[61] + b2Vec2(0, 8.48f);
			}
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

		// gallery ceiling
		{
			float ceilingAngle = atan2(p[54].x - p[52].x, p[52].y - p[54].y) - asin(galleryCeilingOffset / sqrt((p[54] - p[52]).LengthSquared()));
			p[84] = p[52] + galleryCeilingOffset * b2Vec2(cos(ceilingAngle), sin(ceilingAngle));
			p[85] = p[54] - galleryCeilingOffset * b2Vec2(cos(ceilingAngle), sin(ceilingAngle));
		}

		// gallery floor
		{
			p[90] = p[23] + b2Vec2(0, cubit / cos(ascendingAngle));
			p[91] = p[55] + b2Vec2(0, cubit / cos(ascendingAngle));
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
			NextTo LowerChamber_14[3]{
				b2Vec2(border0_top * sin(angle0),-border0_top * cos(angle0)),
				b2Vec2(-(8.27f - 3.21f),0),
				b2Vec2(0, 0)
			};

			p[86] = drawPath(body, p[14], LowerChamber_14);

			NextTo LowerChamber_13[3]{
				b2Vec2(-border0_bottom * sin(descendingAngle),border0_bottom * cos(descendingAngle)),
				b2Vec2(-8.91 - 8.28,0),
				b2Vec2(0, 0)
			};
			p[88] = drawPath(body, p[13], LowerChamber_13);

			NextTo LowerChamber_88[5]{
				b2Vec2(0,2.19f  + 0.91f),
				b2Vec2(8.36f,0),
				b2Vec2(0,-2.19f),
				b2Vec2(8.78 - 7.39,0),
				b2Vec2(0, 0)
			};

			p[87] = drawPath(body, p[88], LowerChamber_88);

			drawAbsorbContainer(body, p[86], p[87]);
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
			drawAbsorbContainer(body, p[44], p[43]);
			drawAbsorbContainer(body, p[38], p[39]);
			drawAbsorbContainer(body, p[42], b2Vec2(p[40].x,p[42].y));
		}

		// Gallery
		// https://upload.wikimedia.org/wikipedia/commons/c/c6/PSM_V80_D462_Longitudinal_sections_of_the_grand_gallery.png

		{
			// Right gallery wall
			{
				float angle = 0;  // Horizontal

				if (rightGalleryWallMode == Parallel) {
					angle = ascendingAngle;
				}

				drawLine(body, p[21], b2Vec2(p[21].x, p[63].y - (p[21].x - p[63].x) * tan(angle)));
				drawLine(body, b2Vec2(p[21].x, p[63].y - (p[21].x - p[63].x) * tan(angle)), p[63]);

				drawLine(body, p[63], b2Vec2(p[63].x, p[64].y - (p[63].x-p[64].x)*tan(angle)));
				drawLine(body, b2Vec2(p[63].x, p[64].y - (p[63].x - p[64].x) * tan(angle)), p[64]);

				drawLine(body, p[64], b2Vec2(p[64].x, p[65].y - (p[64].x-p[65].x)*tan(angle)));
				drawLine(body, b2Vec2(p[64].x, p[65].y - (p[64].x - p[65].x) * tan(angle)), p[65]);

				drawLine(body, p[65], b2Vec2(p[65].x, p[66].y - (p[65].x-p[66].x)*tan(angle)));
				drawLine(body, b2Vec2(p[65].x, p[66].y - (p[65].x - p[66].x) * tan(angle)), p[66]);

				drawLine(body, p[66], b2Vec2(p[66].x, p[67].y - (p[66].x-p[67].x)*tan(angle)));
				drawLine(body, b2Vec2(p[66].x, p[67].y - (p[66].x - p[67].x) * tan(angle)), p[67]);

				drawLine(body, p[67], b2Vec2(p[67].x, p[68].y - (p[67].x - p[68].x) * tan(angle)));
				drawLine(body, b2Vec2(p[67].x, p[68].y - (p[67].x - p[68].x) * tan(angle)), b2Vec2(p[61].x, p[68].y));

				drawLine(body, p[68], p[54]);
			}

			// Left gallery wall
			{
				float angle = 0;  // Horizontal

				if (leftGalleryWallMode == Parallel) {
					angle = ascendingAngle;
				}

				drawLine(body, p[50], b2Vec2(p[50].x, p[77].y+ (p[77].x-p[50].x)*tan(angle)));
				drawLine(body, b2Vec2(p[50].x, p[77].y + (p[77].x - p[50].x) * tan(angle)), p[77]);

				drawLine(body, p[77], b2Vec2(p[77].x, p[78].y + (p[78].x - p[77].x) * tan(angle)));
				drawLine(body, b2Vec2(p[77].x, p[78].y + (p[78].x - p[77].x) * tan(angle)), p[78]);

				drawLine(body, p[78], b2Vec2(p[78].x, p[79].y + (p[79].x - p[78].x) * tan(angle)));
				drawLine(body, b2Vec2(p[78].x, p[79].y + (p[79].x - p[78].x) * tan(angle)), p[79]);

				drawLine(body, p[79], b2Vec2(p[79].x, p[80].y + (p[80].x - p[79].x) * tan(angle)));
				drawLine(body, b2Vec2(p[79].x, p[80].y + (p[80].x - p[79].x) * tan(angle)), p[80]);

				drawLine(body, p[80], b2Vec2(p[80].x, p[81].y + (p[81].x - p[80].x) * tan(angle)));
				drawLine(body, b2Vec2(p[80].x, p[81].y+ (p[81].x - p[80].x) * tan(angle)), p[81]);

				drawLine(body, p[81], b2Vec2(p[81].x, p[82].y+ (p[82].x - p[81].x) * tan(angle)));
				drawLine(body, b2Vec2(p[81].x, p[82].y+ (p[82].x - p[81].x) * tan(angle)), p[82]);

				drawLine(body, p[82], b2Vec2(p[82].x, p[83].y+ (p[83].x - p[82].x) * tan(angle)));
				drawLine(body, b2Vec2(p[82].x, p[83].y+ (p[83].x - p[82].x) * tan(angle)), p[83]);

				drawLine(body, p[83], p[52]);
			}

			// Gallery ceiling
			{
				if (galleryCeilingOffset == 0) {
					drawLine(body, p[52], p[54]);
				}
				else {
					float numberOfSteps = 36;
					float stepWidth = (sqrt((p[52] - p[85]).LengthSquared()) - GALLERY_CEILING_FIRST_STEP_WIDTH) / numberOfSteps;
					//float numberOfSteps = ceil(((sqrt((p[52] - p[85]).LengthSquared()) - GALLERY_CEILING_FIRST_STEP_WIDTH) / 1.2f));
					float angle = atan2(p[52].y - p[85].y, p[85].x - p[52].x);
					float stepHight = sqrt((p[84] - p[52]).LengthSquared()) / numberOfSteps;

					b2Vec2 previousPoint = p[52];

					for (int i = 0;i < numberOfSteps;i++) {
						b2Vec2 p1 = p[52] + (GALLERY_CEILING_FIRST_STEP_WIDTH + stepWidth * i) * b2Vec2(cos(angle), -sin(angle));
						b2Vec2 p2 = p[84] + (GALLERY_CEILING_FIRST_STEP_WIDTH + stepWidth * i) * b2Vec2(cos(angle), -sin(angle));
						b2Vec2 p3 = p[52] + (stepHight * i) * b2Vec2(sin(angle), cos(angle));
						b2Vec2 p4 = p[85] + (stepHight * i) * b2Vec2(sin(angle), cos(angle));
						b2Vec2 p = crossPoint(p1, p2, p3, p4);
						b2Vec2 p_new = p + stepHight * b2Vec2(sin(angle), cos(angle));
						drawLine(body, previousPoint, p);
						drawLine(body, p, p_new);
						previousPoint = b2Vec2(p_new.x, p_new.y);
					}
					drawLine(body, previousPoint, p[54]);
				}
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

			// Gallery beams
			{
					float stepSize = 6.526f * sqrt((p[90] - p[91]).LengthSquared()) / 88.036f;

					for (int i = 0;i < 14;i++) {
						// small hole
						b2Vec2 step_i = p[91] + stepSize * i * b2Vec2(-cos(ascendingAngle), sin(ascendingAngle));

						b2Vec2 p0 = step_i + cubit * b2Vec2(-sin(ascendingAngle), -cos(ascendingAngle));
						
						b2Vec2 p1 = step_i + GALLERY_HOLE_SHORT_DEPTH * b2Vec2(-sin(ascendingAngle), -cos(ascendingAngle));
						b2Vec2 p3 = step_i + (GALLERY_HOLE_SHORT_WIDTH_MUL * stepSize) * b2Vec2(-cos(ascendingAngle), sin(ascendingAngle));
						b2Vec2 p2 = p3 + GALLERY_HOLE_SHORT_DEPTH * b2Vec2(-sin(ascendingAngle), -cos(ascendingAngle));
						b2Vec2 p4 = p3 + GALLERY_HOLES_SPACE_MUL * stepSize * b2Vec2(-cos(ascendingAngle), sin(ascendingAngle));
						b2Vec2 p5 = p4 + GALLERY_HOLE_LONG_DEPTH * b2Vec2(-sin(ascendingAngle), -cos(ascendingAngle));
						b2Vec2 p7 = p4 + (GALLERY_HOLE_LONG_WIDTH_MUL * stepSize) * b2Vec2(-cos(ascendingAngle), sin(ascendingAngle));
						b2Vec2 p6 = p7 + GALLERY_HOLE_LONG_DEPTH * b2Vec2(-sin(ascendingAngle), -cos(ascendingAngle));

						if (i > 0) {
							// short cutting
							b2Vec2 p8 = step_i + GALLERY_HOLE_SHORT_WIDTH_MUL * stepSize * b2Vec2(0, 0.19f);
							b2Vec2 p9 = p8 + (GALLERY_HOLE_SHORT_WIDTH_MUL * stepSize) * b2Vec2(-cos(ascendingAngle), sin(ascendingAngle));
							b2Vec2 p10 = p9 + GALLERY_HOLE_SHORT_WIDTH_MUL * stepSize * b2Vec2(0, 0.38f);
							b2Vec2 p11 = p8 + GALLERY_HOLE_SHORT_WIDTH_MUL * stepSize * b2Vec2(0, 0.38f);

							if (galleryBeamsMode == Reflect) {
								drawLine(body, p8, p9);
								drawLine(body, p9, p10);
								drawLine(body, p10, p11);
								drawLine(body, p11, p8);
							}

							if (galleryBeamsMode == Absorb) {
								drawAbsorbLine(body, p8, p9);
								drawAbsorbLine(body, p9, p10);
								drawAbsorbLine(body, p10, p11);
								drawAbsorbLine(body, p11, p8);
							}

							if (i < 13) {
								// long cutting
								b2Vec2 p12 = p4 + GALLERY_HOLE_SHORT_WIDTH_MUL * stepSize * b2Vec2(0, 0.19f);
								b2Vec2 p13 = p12 + (GALLERY_HOLE_LONG_WIDTH_MUL * stepSize) * b2Vec2(-cos(ascendingAngle), sin(ascendingAngle));
								b2Vec2 p14 = p13 + GALLERY_HOLE_SHORT_WIDTH_MUL * stepSize * b2Vec2(0, 0.38f);
								b2Vec2 p15 = p12 + GALLERY_HOLE_SHORT_WIDTH_MUL * stepSize * b2Vec2(0, 0.38f);

								if (galleryBeamsMode == Reflect) {
									drawLine(body, p12, p13);
									drawLine(body, p13, p14);
									drawLine(body, p14, p15);
									drawLine(body, p15, p12);
								}

								if (galleryBeamsMode == Absorb) {
									drawAbsorbLine(body, p12, p13);
									drawAbsorbLine(body, p13, p14);
									drawAbsorbLine(body, p14, p15);
									drawAbsorbLine(body, p15, p12);
								}
							}
						}
				}

			
			
			
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

	
};

static int testIndex = RegisterTest("Pyramid", "Pyramid", Piramid::Create);