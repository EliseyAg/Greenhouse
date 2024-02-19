#include "core0.h"

TaskHandle_t Task0;
StaticQueue_t _dataSendQueueBuffer;
uint8_t _dataSendQueueStorage[10 * sizeof(int)];
QueueHandle_t _dataSendQueue = xQueueCreateStatic(10, sizeof(int), &(_dataSendQueueStorage[0]), &_dataSendQueueBuffer);

int a;

void core0(void *p)
{
  int item = 10;
  for(;;)
  {
    xQueueSend(_dataSendQueue, &item, pdMS_TO_TICKS(1000));
    item++;
  }
}

void setup()
{
  xTaskCreatePinnedToCore(core0, "Task0", 10000, NULL, 1, &Task0, 0);

  Serial.begin(9600);
}

void loop()
{
  xQueueReceive(_dataSendQueue, &a, 0);
  Serial.println(a);
}
