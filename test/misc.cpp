#include "game.h"

#include <gtest/gtest.h>

#include "Serialization/IOStreamBuffers.h"
#include "Serialization/StringBuffers.h"
#include "Machine/IMachine.h"
#include "Geometry/ShapeConcept.h"
#include "Geometry/Circle.h"
#include "Geometry/Triangle.h"
#include "Geometry/Segment.h"
#include "Geometry/Polygon.h"
#include "Geometry/Ellipse.h"
#include "Geometry/Capsule.h"
#include "Geometry/Discretize.h"
#include "Geometry/RandomPoint.h"
#include "Geometry/RayCast.h"
#include "Geometry/Collision.h"

using namespace gamelib;
using namespace glm;

template <shape T>
void test_shape(T& s)
{
	using value_type = typename T::value_type;
	EXPECT_TRUE(s.is_valid());

	std::default_random_engine rng{};
	for (int i = 0; i < 100; ++i)
	{
		EXPECT_TRUE((bool)s.contains(random_point(s, rng))) << i;
	}
	
	EXPECT_GT(s.calculate_area(), 0);

	EXPECT_GT(s.edge_length(), 0);
	EXPECT_EQ(s.classify(s.edge_point_alpha(0), 0.00001), point_relationship::on_edge);
	EXPECT_EQ(s.classify(s.edge_point_alpha(1), 0.00001), point_relationship::on_edge);
	for (int i = 0; i < 100; ++i)
	{
		auto rnd = random::Percentage(rng);
		auto p = s.edge_point_alpha(rnd);
		EXPECT_EQ(s.classify(p, 0.00001), point_relationship::on_edge) << i << p << rnd;
	}

	const auto bb = s.bounding_box();
	EXPECT_TRUE(bb.is_valid());
	for (int i = 0; i < 100; ++i)
	{
		EXPECT_TRUE(bb.contains(random_point(s, rng))) << i;
	}

	for (int i = 0; i < 100; ++i)
	{
		tvec2<value_type> p = { random::RealRange(rng, -100.0, 100.0), random::RealRange(rng, -100.0, 100.0) };
		const auto projected = s.projected(p);
		EXPECT_EQ(s.classify(projected, 0.00001), point_relationship::on_edge) << i << " " << projected;
	}

	auto p = random_point(tvec2<value_type>{ 10.0,10.0 }, rng);
	s.set_position(p);
	EXPECT_EQ(s.position(), p);

	{
		T s{};
		EXPECT_FLOAT_EQ(s.calculate_area(), 0);
		EXPECT_EQ(s.classify({}), point_relationship::on_edge);
		EXPECT_FLOAT_EQ(s.edge_length(), 0);
	}
}

TEST(shapes, circle)
{
	tcircle2<double> s{ {-23.0f, 15.0f}, 10.0f };
	test_shape(s);
	EXPECT_FLOAT_EQ(s.radius_squared(), s.radius * s.radius);

	auto unit_circle = tcircle2<double>{ {}, 1.0f };
	EXPECT_EQ((unit_circle.classify({ 1.0f, 0.0f }, 0.00001)), point_relationship::on_edge);
	EXPECT_EQ((unit_circle.classify({ 0.9f, 0.0f })), point_relationship::inside);
	EXPECT_EQ((unit_circle.classify({ 1.9f, 0.0f })), point_relationship::outside);

	auto pos = unit_circle.edge_point(unit_circle.edge_length() * 0.25);
	EXPECT_EQ(pos, unit_circle.edge_point_alpha(0.25));
	EXPECT_TRUE(epsilonEqual(pos.x, 0.0, 0.00001)) << pos.x;
	EXPECT_TRUE(epsilonEqual(pos.y, 1.0, 0.00001));

	{
		tcircle2<double> s{ {-23.0, 15.0}, -10.0 };
		EXPECT_FALSE(s.is_valid());
		s.make_valid();
		EXPECT_TRUE(s.is_valid());
	}
}

TEST(shapes, rect)
{
	rec2 s{ {-23.0f, 15.0f}, {10.0f, 56.0f} };
	test_shape(s);

	{
		rec2 s{ {-23.0f, 15.0f}, {10.0f, -56.0f} };
		EXPECT_FALSE(s.is_valid());
		s.make_valid();
		EXPECT_TRUE(s.is_valid());
	}
}
