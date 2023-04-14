/*
接线说明:无

程序说明:开发板订阅多个主题,同时使用单级和多级通配符匹配订阅的主题,
        当订阅的任一主题发布1或0会对灯有影响(当发布的信息首部为1或0也有控制效果)
        使用服务质量等级来订阅主题(QoS1)
        使用这个QOS1最直接的效果是当开发板处于断电情况下订阅的主题发布的消息会在开发板上电后接收到,
        (在程序中只有一个订阅的主题使用了该服务,就是该主题:lingsou-F4:12:FA:E2:60:98)


注意事项:尽量不要在loop()函数中使用延时函数,可能会对MQTT服务有影响,所以使用 #include <Ticker.h> 这个库来定时执行任务,以替代延时

函数示例:无

作者:灵首

时间:2023_4_14

*/
#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiMulti.h>


WiFiMulti wifi_multi; // 建立WiFiMulti 的对象,对象名称是 wifi_multi
WiFiClient wifiClient;  //建立WiFiClient
PubSubClient mqttClient(wifiClient);  //根据WiFiClient来建立PubSubClient对象


const char* mqttServer = "test.ranye-iot.net";  //这是需要连接的MQTT服务器的网址,可更改
const int subQoS = 1;     // 客户端订阅主题时使用的QoS级别（截止2020-10-07，仅支持QoS = 1，不支持QoS = 2）
const bool cleanSession = false; // 清除会话（如QoS>0必须要设为false）
 
const char* willTopic = "willTopic"; // 遗嘱主题名称
const char* willMsg = "willMsg";     // 遗嘱主题信息
const int willQos = 0;               // 遗嘱QoS
const int willRetain = false;        // 遗嘱保留

#define LED_A 10
#define LED_B 11



/**
* @brief 连接WiFi的函数
*
* @param 无
* @return 无
*/
void wifi_multi_con(void)
{
  int i = 0;
  while (wifi_multi.run() != WL_CONNECTED)
  {
    delay(1000);
    i++;
    Serial.print(i);
  }
}



/**
* @brief 写入自己要连接的WiFi名称及密码,之后会自动连接信号最强的WiFi
*
* @param 无
* @return 无
*/
void wifi_multi_init(void)
{
  wifi_multi.addAP("haoze2938", "12345678");
  wifi_multi.addAP("LINGSOU1029", "12345678");
  wifi_multi.addAP("haoze1029", "12345678"); // 通过 wifi_multi.addAP() 添加了多个WiFi的信息,当连接时会在这些WiFi中自动搜索最强信号的WiFi连接
}



/**
* @brief 订阅相关的主题(一共订阅了3个主题),一个普通主题,一个使用单级通配符,一个使用多级通配符
*
* @param 无
* @return 无
*/
void subscribleTopic(){
  // 建立订阅主题1。主题名称根据发布消息的开发板决定
  // 这么做是为确保不同设备使用同一个MQTT服务器测试消息订阅时，所订阅的主题名称不同
  //需要将字符串转换为字符数组满足库的要求
  //这个主题由另一个开发板建立并发送
  String topicString1 = "lingsou-F4:12:FA:E2:60:98";
  char subTopic1[topicString1.length() + 1];  
  strcpy(subTopic1, topicString1.c_str());
  
  // 建立订阅主题2,使用单级通配符
  //这个主题由MQTTfx建立并发送
  String topicString2 = "lingsou-" + WiFi.macAddress() + "/+/data";
  char subTopic2[topicString2.length() + 1];  
  strcpy(subTopic2, topicString2.c_str());

  // 建立订阅主题3,使用多级通配符
  //这个主题由MQTTfx建立并发送
  String topicString3 = "lingsou-" + WiFi.macAddress() + "/sensor/#";
  char subTopic3[topicString3.length() + 1];  
  strcpy(subTopic3, topicString3.c_str());
  
  // 通过串口监视器输出是否成功订阅主题1以及订阅的主题1名称
  if(mqttClient.subscribe(subTopic1,subQoS)){
    Serial.println("Subscrib Topic:");
    Serial.println(subTopic1);
    Serial.print("\n");
  } else {
    Serial.print("Subscribe Fail...");
    Serial.print("\n");
  }  
 
  // 通过串口监视器输出是否成功订阅主题2以及订阅的主题2名称
  if(mqttClient.subscribe(subTopic2)){
    Serial.println("Subscrib Topic:");
    Serial.println(subTopic2);
    Serial.print("\n");
  } else {
    Serial.print("Subscribe Fail...");
    Serial.print("\n");
  }

  // 通过串口监视器输出是否成功订阅主题3以及订阅的主题3名称
  if(mqttClient.subscribe(subTopic3)){
    Serial.println("Subscrib Topic:");
    Serial.println(subTopic3);
    Serial.print("\n");
  } else {
    Serial.print("Subscribe Fail...");
    Serial.print("\n");
  }
}



/**
* @brief 生成客户端名称并连接服务器同时订阅主题并串口输出数据
*
* @param 无
* @return 无
*/
void connectMQTTServer(){
  //生成客户端的名称(同一个服务器下不能存在两个相同的客户端名称)
  String clientId = "esp32s3---" + WiFi.macAddress();

  /* 连接MQTT服务器
  boolean connect(const char* id, const char* user, 
                  const char* pass, const char* willTopic, 
                  uint8_t willQos, boolean willRetain, 
                  const char* willMessage, boolean cleanSession); 
  若让设备在离线时仍然能够让qos1工作，则connect时的cleanSession需要设置为false                
                  */
  if(mqttClient.connect(clientId.c_str(), NULL, NULL, willTopic, willQos, willRetain, willMsg, cleanSession)){
    Serial.println("MQTT Server Connect successfully!!!.\n");
    Serial.println("Server Address: ");
    Serial.println(mqttServer);
    Serial.print("\n");
    Serial.println("ClientId:");
    Serial.println(clientId);
    Serial.print("\n");
    
    //连接服务器后订阅主题
    subscribleTopic();
  }
  else{
    Serial.print("MQTT Server Connect Failed. Client State:");
    Serial.println(mqttClient.state());
    Serial.print("\n");
    delay(3000);
  }

}



/**
* @brief 这是一个回调函数,当订阅的主题有消息发布时就会调用该函数,参数是固定的(PunSubClient中固定的),不能自己修改
*
* @param char* topic :这是订阅的主题名
* @param byte* payload :这是传回的消息
* @param unsigned int length :这是消息长度
* @return 无
*/
void recieveCallback(char* topic, byte* payload, unsigned int length){
  //输出订阅的主题名称
  Serial.print("Message Received [");
  Serial.print(topic);
  Serial.print("] ");
  Serial.print("\n");

  //输出订阅的主题中的消息
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println("");
  Serial.print("Message Length(Bytes) ");
  Serial.println(length);
  Serial.print("\n");

 //根据主题信息控制LED灯的亮灭
  if ((char)payload[0] == '1') {     // 如果收到的信息以“1”为开始
    digitalWrite(LED_A, 1);  // 则点亮LED。
    digitalWrite(LED_B, 1);  // 则点亮LED。
  } else {                           
    digitalWrite(LED_A, 0); // 否则熄灭LED。
    digitalWrite(LED_B, 0); // 否则熄灭LED。
  }

}


void setup() {
  // 连接串口
  Serial.begin(9600);
  Serial.print("serial is OK\n");

  //led灯设置
  pinMode(LED_A,OUTPUT);
  pinMode(LED_B,OUTPUT);
  digitalWrite(LED_A,0);
  digitalWrite(LED_B,0);

  // wifi 连接设置
  wifi_multi_init();
  wifi_multi_con();
  Serial.print("wifi connected!!!\n");

  // 输出连接信息(连接的WIFI名称及开发板的IP地址)
  Serial.print("\nconnect wifi:");
  Serial.print(WiFi.SSID());
  Serial.print("\n");
  Serial.print("\nIP address:");
  Serial.print(WiFi.localIP());
  Serial.print("\n");

  //设置连接的MQTT服务器
  mqttClient.setServer(mqttServer,1883);

  //设置回调函数处理订阅消息
  mqttClient.setCallback(recieveCallback);

  //连接MQTT服务器
  connectMQTTServer();
}



void loop() {
  //检查MQTT连接,若连接则保持心跳
  //若未服务器未连接则尝试重连
  if(mqttClient.connected()){
    mqttClient.loop();  //这是在保持客户端心跳
  }
  else{
    connectMQTTServer();  //重连服务器
  }
}