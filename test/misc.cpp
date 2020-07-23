#include "game.h"
#include <gtest/gtest.h>

struct DummyErrorReporter : IErrorReporter
{
	mutable gamelib::ReportType type;
	virtual void PerformReport(Reporter const& holder) const override
	{
		type = holder.Type;
	}
};

TEST(container, RegisterInstance)
{
	gamelib::ioc::Container container;
	DummyErrorReporter err;
	container.RegisterInstance<IErrorReporter>(&err);
	auto err_rep = container.GetInstance<IErrorReporter>();
	err_rep->Warning("yay");

	EXPECT_EQ(err.type, gamelib::ReportType::Warning);
}

TEST(container, RegisterConcreteType)
{
	gamelib::ioc::Container container;
	container.RegisterConcreteType<IErrorReporter, DummyErrorReporter>();
	auto err_rep = container.GetInstance<IErrorReporter>();
	auto err_rep2 = container.GetInstance<IErrorReporter>();
	EXPECT_NE(err_rep, err_rep2);
}

TEST(container, RegisterFactory)
{
	gamelib::ioc::Container container;
	DummyErrorReporter err;
	container.RegisterFactory<IErrorReporter>([&err]() { return std::shared_ptr<IErrorReporter>{ std::shared_ptr<void>{}, &err }; });
	auto err_rep = container.GetInstance<IErrorReporter>();
	err_rep->Warning("yay");

	EXPECT_EQ(err.type, gamelib::ReportType::Warning);
}
