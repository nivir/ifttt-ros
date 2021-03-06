#include "trigger_event_publisher.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <std_msgs/String.h>

using namespace ifttt;

struct EncodeStringMsg
{
  virtual ~EncodeStringMsg() = default;
  virtual TriggerEvent toTriggerEvent(const std_msgs::String & msg) = 0;
};

struct MockEncodeStringMsg : public EncodeStringMsg
{
  MOCK_METHOD1(toTriggerEvent, TriggerEvent(const std_msgs::String & msg));
};

TEST(TriggerEventPublisherTest, TriggerEventEncoder_IsCalled)
{
  using std::placeholders::_1;
  using testing::_;
  using testing::AtLeast;
  using testing::DefaultValue;

  TriggerEvent event;
  event.header.stamp = ros::Time::now();
  DefaultValue<TriggerEvent>::Set(event);

  MockEncodeStringMsg mock_encoder;
  EXPECT_CALL(mock_encoder, toTriggerEvent(_)).Times(AtLeast(1));

  TriggerEventEncoder<std_msgs::String> callback = std::bind(&MockEncodeStringMsg::toTriggerEvent, &mock_encoder, _1);

  std::string source_topic_name;
  if (!ros::param::get("/source_topic_name", source_topic_name))
  {
    FAIL() << "Please ensure source_topic_name parameter is set for this test";
  }

  auto test_publisher = TriggerEventPublisher<std_msgs::String>(source_topic_name, 10, callback, "/test_events", 10);

  auto deadline = ros::Time::now() + ros::Duration(1);
  ros::Rate loop_rate(10);
  while (ros::ok() && ros::Time::now() < deadline)
  {
    ros::spinOnce();
    loop_rate.sleep();
  }

  DefaultValue<TriggerEvent>::Clear();
}

int main(int argc, char ** argv)
{
  ros::init(argc, argv, "test_trigger_event_publisher");
  ros::NodeHandle nh;
  testing::InitGoogleMock(&argc, argv);
  return RUN_ALL_TESTS();
}