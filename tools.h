#include "settings.h"
#include "test.h"
#include "imgui/imgui.h"

#include <string>

#define PI 3.14159265f
#define c 299792458                                                          // Speed of light in vacuum
constexpr auto cubit = PI / 6;

class RayCastClosestCallback : public b2RayCastCallback
{
public:
	RayCastClosestCallback()
	{
		m_hit = false;
	}

	float ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float fraction) override
	{
		m_fixture = fixture;
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
	b2Fixture* m_fixture;
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
	int maximumReflections = 300;

	Ray(b2Vec2 _from, float _length, float _angle) {
		from = b2Vec2(_from.x, _from.y);
		length = _length;
		angle = _angle;
	}

	Ray(b2Vec2 _from, float _length, float _angle, int _maxReflections) {
		from = b2Vec2(_from.x, _from.y);
		length = _length;
		angle = _angle;
		maximumReflections = _maxReflections;
	}
};

inline void angle2minutesAndSeconds(char *buffer,int size, char *prefix,float angle) {
	float a = 180 * angle / PI;
	
	int degrees = floor(a);
	int minutes = 60.f * (a - floor(a));
	int seconds = floor((a-floor(a) - (float)minutes/60.f)*3600.f);

	snprintf(buffer, size, "%s %d.%d'%d\"", prefix,degrees,minutes,seconds);
}

inline b2Vec2 reflect(b2Vec2 vector, b2Vec2 normal) {
	float num2 = vector.x * normal.x + vector.y * normal.y;
	return b2Vec2(vector.x - 2.0f * num2 * normal.x, vector.y - 2.0f * num2 * normal.y);
}
inline void drawPoint(b2Vec2 point, char* name) {
	g_debugDraw.DrawCircle(point, 0.1f, b2Color(1, 1, 1));
	g_debugDraw.DrawString(point, name);
}
inline float drawRay(b2World* m_world, Ray ray, b2Color color) {
	float distance = 0;
	// initial source is little bit different to start raycasting from corner

	b2Vec2 source = b2Vec2(ray.from.x + 0.01f * cos(ray.angle), ray.from.y + 0.01f * sin(ray.angle));


	b2Vec2 destination = b2Vec2(ray.from.x + ray.length * cos(ray.angle), ray.from.y + ray.length * sin(ray.angle));

	for (int i = 0;i < ray.maximumReflections + 1;i++) {
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
				distance += sqrt((callback.m_point - source).LengthSquared());
				//g_debugDraw.DrawSegment(callback.m_point, callback.m_point + 0.5 * callback.m_normal, b2Color(1, 0, 0)); // normal

				char str[16];
				float a = atan2(callback.m_point.y - source.y, callback.m_point.x - source.x) * 180 / PI;
				snprintf(str, sizeof(str), "o=%.2f", a);
				//g_debugDraw.DrawString(source, str);

				destination = callback.m_point + ray.length * reflect(callback.m_point - source, callback.m_normal);

				b2Vec2 direction = destination - callback.m_point;
				source = callback.m_point + 0.0001f * b2Vec2(direction.x / direction.Length(), direction.y / direction.Length());

				if (callback.m_fixture->GetUserData() == "absorb") {
					break;
				}

			}
		}
		else {
			//g_debugDraw.DrawCircle(source, 0.5f, b2Color(0, 1, 0));
			//g_debugDraw.DrawString(source, "source=destination");
			break;
		}
	}
	return distance;
}
inline void drawRainbowRay(b2World* m_world,b2Vec2 start, b2Vec2 end, float angle, int rays) {
	for (float i = 0;i < rays;i++) {
		b2Vec2 point = start + (i / (float)rays) * (end - start);

		b2Color color = b2Color(
			0.5 + cos(5 * i / rays * 2 * PI / 6) / 2,
			0.5 + cos(5 * i / rays * 2 * PI / 6 + 2 * PI / 3) / 2,
			0.5 + cos(5 * i / rays * 2 * PI / 6 + 4 * PI / 3) / 2
		);
		drawRay(m_world,Ray(point, 100, angle), color);
	}
}
inline b2Vec2 drawPath(b2Body* body, b2Vec2 startPoint, NextTo* path) {
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
inline b2Vec2 drawLine(b2Body* body, b2Vec2 startPoint, b2Vec2 endPoint) {
	b2EdgeShape shape;
	b2FixtureDef fd;
	fd.shape = &shape;
	fd.density = 0.0f;
	fd.friction = 0.6f;
	shape.SetTwoSided(startPoint, endPoint);
	body->CreateFixture(&fd);
	return endPoint;
};
inline b2Vec2 drawAbsorbLine(b2Body* body, b2Vec2 startPoint, b2Vec2 endPoint) {
	b2EdgeShape shape;
	b2FixtureDef fd;
	fd.shape = &shape;
	fd.density = 0.0f;
	fd.friction = 0.6f;
	fd.userData = "absorb";
	shape.SetTwoSided(startPoint, endPoint);
	body->CreateFixture(&fd);
	return endPoint;
};
inline void drawAbsorbContainer(b2Body* body, b2Vec2 startPoint, b2Vec2 endPoint) {
	constexpr auto absorbContainerDepth = 0.4f;

	float angle = atan2(endPoint.y - startPoint.y, endPoint.x - startPoint.x) - PI / 2;

	b2Vec2 p1 = startPoint + absorbContainerDepth * b2Vec2(cos(angle), sin(angle));
	b2Vec2 p2 = endPoint + absorbContainerDepth * b2Vec2(cos(angle), sin(angle));

	drawAbsorbLine(body, startPoint, p1);
	drawAbsorbLine(body, endPoint, p2);
	drawAbsorbLine(body, p1, p2);
}
