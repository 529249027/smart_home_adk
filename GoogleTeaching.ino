#include <usbhub.h>
#include <adk.h>

#define COMMAND_TYPE_LED	0x01
#define COMMAND_FOUND		0x07
#define COMMAND_UPDATE		0x09
#define COMMAND_SETTING		0x0A

//https://github.com/felis/USB_Host_Shield_2.0

USB Usb;
USBHub hub0(&Usb);
USBHub hub1(&Usb);
ADK adk(&Usb, "Dongying Vocational Institute",
	"AndroidStudy",
	"Connect Zigbee 2530 and Control Sensors",
	"1.0",
	"http://www.dyxy.net",
	"20160907");

uint8_t sended = 0;

/*
 * 处理平板发给ADK的命令
 * 命令格式：len	cmd		data
 *           len：数据长度1字节
 *			 cmd：命令1字节
 *			 data：数据，len字节
*/
void handleAdkSettingCommand(uint8_t* buffer) {

}

uint8_t CalcFCS(uint8_t* buffer, uint8_t len)
{
	uint8_t xorResult = 0;
	for (uint8_t x = 0; x < len; x++, buffer++) {
		xorResult = xorResult ^ *buffer;
	}

	return (xorResult);
}


void setup()
{
	Serial1.begin(9600);
	while (!Serial1);
	/*Serial.begin(115200);
	while (!Serial);*/

	//Serial.println("Serial OK");

	if (Usb.Init() == -1) {
		//Serial.println("OSCOKIRQ failed to assert");
		while (1); //halt
	}
}

void loop()
{
	Usb.Task();

	if (adk.isReady() == false) {
		//Serial.println("adk not ready");
		delay(500);
		return;
	}

	uint8_t receivedBuffer[512];
	uint16_t len = sizeof(receivedBuffer) / sizeof(uint8_t);
	uint8_t ch;

	//Serial.println("Point1");

	//协调发发送过来数据
	while ((Serial1.available() > 0) && ((ch = Serial1.read()) != 0xfe));
	//等到命令开始的字符
	if (ch == 0xfe) {//一条命令
		memset(receivedBuffer, 0, len);

		receivedBuffer[0] = 0xfe;

		while (Serial1.available() == 0);
		receivedBuffer[1] = Serial1.read();//长度

		Serial1.readBytes(&receivedBuffer[2], receivedBuffer[1] + 3);

		ch = CalcFCS(&receivedBuffer[1], receivedBuffer[1] + 3);
		if (ch == receivedBuffer[receivedBuffer[1] + 4]) {//验证得到正确的数据
			//Serial.write(receivedBuffer, len);
			//Serial.println("ZIGBEE->ADK");

			adk.SndData(receivedBuffer[1] + 5, receivedBuffer);

			//delay(1000);
		}
	}

	//ADK发送过来的数据
	uint8_t rcode;
	memset(receivedBuffer, 0, len);

	//Serial.println("Point2");

	rcode = adk.RcvData(&len, receivedBuffer);
	if (len > 0) {
		uint16_t index = 0;
		while (index < len) {
			if (receivedBuffer[index] == 0xfe) {
				int dataLen = receivedBuffer[index + 1];
				uint8_t* buffer = (uint8_t*)(malloc((dataLen + 5)*sizeof(uint8_t)));

				memcpy(buffer, &receivedBuffer[index], dataLen + 5);

				index += 1 + 1 + 2 + dataLen + 1;

				//验证是合法的命令
				if (buffer[dataLen + 4] == CalcFCS(&buffer[1], dataLen + 3)) {
					//Serial.println("ADK->ZIGBEE");

					//Serial.write(buffer, dataLen + 5);

					uint16_t command = (buffer[2] << 8) + buffer[3];

					if (command == 0x0100) {
						handleAdkSettingCommand(buffer);
					}
					else {
						////产生设备信息
						//uint8_t bs1[] = { 0xfe, 0x0c, 0x00, 0x01, 0x01, 0x01, 0x01 ,0x01, 0x01, 0x01 ,0x01, 0x01, 0x02, 0x04, 0x00, 0x0c, 0x07 };
						//uint8_t bs2[] = { 0xfe, 0x0c, 0x00, 0x01, 0x02, 0x02, 0x02 ,0x02, 0x02, 0x02 ,0x02, 0x02, 0x02, 0x02, 0x00, 0x01, 0x0c };

						//adk.SndData(sizeof(bs1) / sizeof(uint8_t), bs1);
						//delay(2500);
						//adk.SndData(sizeof(bs2) / sizeof(uint8_t), bs2);
						//delay(2500);

						//Serial.println("find Device");
						Serial1.write(buffer, dataLen + 5);
						Serial1.flush();
					}
				}
			}
			else {
				index++;
			}
		}
	}

	delay(100);
	//Serial.println("Point3");
}