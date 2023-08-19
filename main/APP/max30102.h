#ifndef max30102_h
#define max30102_h

typedef struct
{
    float heartrate;
    float pctspo2;
} hr_spo2_t;

void max30102_init();
void max30102_task(void *pvParameters);


#endif

