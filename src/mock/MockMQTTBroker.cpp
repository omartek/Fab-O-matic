#include "mock/MockMQTTBroker.hpp"
#include "conf.hpp"

namespace fablabbg
{

  MockMQTTBroker::MockMQTTBroker()
  {
  }

  void MockMQTTBroker::start()
  {
    while (WiFi.status() != WL_CONNECTED)
    { // Wait for the Wi-Fi to connect
      if (conf::debug::ENABLE_LOGS)
        Serial.printf("MQTT BROKER: WiFi status changed to %d\r\n", WiFi.status());
      is_running = false;
      return;
    }
    if (!is_running)
    {
      is_running = init(MockMQTTBroker::MQTTPORT, true);

      if (conf::debug::ENABLE_LOGS)
        Serial.printf("MQTT BROKER: started with result %d\r\n", is_running.load());
    }
  }
  bool MockMQTTBroker::onEvent(sMQTTEvent *event)
  {
    switch (event->Type())
    {
    case NewClient_sMQTTEventType:
    {
      sMQTTNewClientEvent *e = (sMQTTNewClientEvent *)event;

      if (conf::debug::ENABLE_LOGS)
        Serial.printf("MQTT BROKER: client connected, id:%s\r\n", e->Client()->getClientId().c_str());
    }
    break;
    case Public_sMQTTEventType:
    {
      sMQTTPublicClientEvent *e = (sMQTTPublicClientEvent *)event;
      topic = e->Topic();
      payload = e->Payload();

      if (conf::debug::ENABLE_LOGS)
        Serial.printf("MQTT BROKER: Received  %s -> %s\r\n", topic.c_str(), payload.c_str());
      // Call the configured replies generation callback
      std::string reply = "";
      while (true)
      {
        if (lock())
        {
          reply = callback(payload);
          unlock();
          break;
        }
      }
      std::string topic_reply = topic + "/reply";
      publish(topic_reply, reply, 0, false);
    }
    break;
    case RemoveClient_sMQTTEventType:
    {
      sMQTTRemoveClientEvent *e = (sMQTTRemoveClientEvent *)event;

      if (conf::debug::ENABLE_LOGS)
        Serial.printf("MQTT BROKER: removed client id: %s\r\n", e->Client()->getClientId().c_str());
    }
    break;
    case LostConnect_sMQTTEventType:
      is_running = false;
      break;
    }
    return true;
  }

  bool MockMQTTBroker::isRunning() const
  {
    return is_running;
  }

  /// @brief Returns a fake server reply for testing purposes
  /// @return json payload
  std::string MockMQTTBroker::defaultReplies(const std::string &query) const
  {
    if (query.find("checkmachine") != std::string::npos)
    {
      return "{\"request_ok\":true,\"is_valid\":true,\"allowed\":true,\"maintenance\":false,\"logoff\":30,\"name\":\"ENDER_1\",\"type\":1}";
    }

    if (query.find("maintenance") != std::string::npos)
    {
      return "{\"request_ok\":true}";
    }

    if (query.find("startuse") != std::string::npos)
    {
      return "{\"request_ok\":true}";
    }

    if (query.find("stopuse") != std::string::npos)
    {
      return "{\"request_ok\":true}";
    }

    if (query.find("checkuser") != std::string::npos)
    {
      auto uid = query.substr(query.find("uid") + 6 + 4, 4);
      auto response = "{\"request_ok\":true,\"is_valid\":true,\"level\":2,\"name\":\"USER" + uid + "\",\"is_valid\":true}";
      return response;
    }

    return "{\"request_ok\":true}";
  }

  void MockMQTTBroker::configureReplies(std::function<std::string(std::string)> callback)
  {
    while (true)
    {
      if (lock())
      {
        this->callback = callback;
        unlock();
        break;
      }
    }
  }
} // namespace fablabbg