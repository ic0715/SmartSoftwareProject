// Do not remove the include below
#include "SmartCar_01.h"
#include "Timer2.h"

/*������ ������ ��Ȯ�� ������ �ϱ� ���� ������*/
char RX_buf[17];
unsigned long int RX_buf2[12]; // ���� 7�� + �Ĺ� 5�� ���� ���� �迭
unsigned long int RX_before[12]={0,};
unsigned long int score[12]={0,}; //����ġ �ݿ�
unsigned char TX_buf0[5] = {0x76, 0x00, 0xF0, 0x00, 0xF0}; // All(���Ĺ� ��� ���� ���)
unsigned char TX_bufs[5] = {0x76, 0x00, 0x0F, 0x00, 0x0F}; //STOP

/*������� ����� �����ϱ� ���ؼ� ����Ʈī�� ȸ������ �����ϱ� ���� ������*/
unsigned long int ENCODER_CNT_L=0, ENCODER_CNT_R=0; //pulse ��  count�� ���� ����
unsigned long int Encoder_value_L=0, Encoder_value_R=0; //���� pulse ���� �����ϴ� ����
#define momentum_value ((double)((Encoder_value_R)/390)*(double)50)/100 //������� ����� ��� �Ÿ� �������� ����Ѵ�. ������ m

/*������� ���� �ӵ��� ���� ����Ʈī�� �ӵ� ��ȭ�� �ֱ� ���� ������*/
unsigned int sensor_sum1[12] = {0,}; //
unsigned int sensor_sum2[12] = {0,};
unsigned int sensor_temp[12] = {0,}; //������ 4�� term ������ �����ϴ� �迭
int sensor_count = 0;

/*�Լ� �ȿ��� ���̴� ������*/
unsigned int RX_min=0;
unsigned int RX_min_index=0;
unsigned int score_max = score[0];
int score_max_index = 0; // getMaxValueIndex���� ���� score_max_index
int RX_index = 0; // switch�� �ȿ� �� ���� ��ȣ

/*pin �� ����*/
int Front_LED = 10;
int Back_LED = 9;
int LED_state=0;
int BUZZER = 45;
int Motor[6] = {22, 23, 24, 25, 4, 5};

int flag = 0; // ���� ������ 1�� �Ͽ� ����Ʈī�� ������ �� �ִ� ������ ���� ���� ����
int start_signal = 0; //������ ���� ���� �޾ƿͼ� start ��ư�� ������ �� ����Ʈī�� ������ �� �ִ� ������ ���� ���� ����, Stop �� ������ 0�� �Ǿ� �����ȿ� ���� ����.

int delay_time =300; //delay_time�� 0.3�ʷ� ����
int PWM_value = 120; //�ӵ��� 120 ���� �ʱ�ȭ

int success=0; // ����ڰ� ������ �������� �� ����Ʈī�� ����� ������ �����ϴ� success ����
unsigned int score_std_value= 25; //score ����ġ ���� �� ���� ���� 25�� �ο�
unsigned long int RX_std_value = 60; // ����ġ�� std_value ���� �Ǿ���, ����ڰ� ����� ������ �ִ��� ������ ������ üũ�ϴ� ���� ���� 60���� �ο�

//The setup function is called once at startup of the sketch
void setup()
{
	// Add your initialization code here
	int z;
	delay(1000);
	Serial1.begin(115200); // ATmega2560 - ATmega128 ���
	Serial.begin(115200); // ATmega2560 - Android Device ���

	pinMode(Front_LED,OUTPUT);
	pinMode(Back_LED,OUTPUT);
	pinMode(BUZZER, OUTPUT);
	LED_set(LED_state);

	Timer2::set(1000000,Timer2_ISR);  // LED�� 1�ʿ� �ѹ��� �����̱� ����  Timer set
	Timer2::start();

	attachInterrupt(6,Encoder_count_L,RISING);
	attachInterrupt(7,Encoder_count_R,RISING);

	for(z=0;z<6;z++){
		pinMode(Motor[z], OUTPUT);
		digitalWrite(Motor[z], LOW);}

	Serial1.write(TX_bufs, 5); //������ ������  STOP ���·� �ʱ�ȭ
	Motor_Control('A', PWM_value);
	Motor_mode(STOP); //����Ʈī ���� ���µ�  STOP���� �ʱ�ȭ
}

// The loop function is called in an endless loop
void loop()
{
	//Add your repeated code here
	if(flag==1 && start_signal==1){ // ������ ������ ������� ������ �����ϰ�(flag=1)&&����ڰ� start ��ư�� ������ ��(start_signal=1)
		success++; //����Ʈī��  �����̰� �����Ƿ� successs value�� ������

		digitalWrite(BUZZER, 0);

		Timer2::stop();
		LED_state=1;
		LED_set(LED_state);  //LED ������ �����ϰ� �׻� ���� ���·� �ٲ���.

		for(int z=0; z<12; z++)
		{ sensor_temp[z] = 0;
		sensor_sum1[z] = 0;
		sensor_sum2[z] = 0;
		}
		sensor_count = 0;   //temp ��, sensor_count �� �ʱ�ȭ

		int temp_speed = PWM_value;  // ȸ���� ���� full speed�� �ֱ� ���ؼ� �ӽ÷� ���� ���ǵ带 �����ϴ� ���� temp_speed
		PWM_value = 250; // ȸ���� ���� speed 250.
		Motor_Control('A', PWM_value);
		switch(RX_index){
		case 0:  //F0
			Motor_mode(RIGHT_U);
			delay(delay_time*2);
			break;
		case 1:  //F1
			Motor_mode(LEFT_U);
			delay(delay_time*3);
			break;
		case 2:  //F2
			Motor_mode(LEFT_U);
			delay(delay_time*3);
			break;
		case 3:  //F3
			Motor_mode(LEFT_U);
			delay(delay_time*4);
			break;
		case 4:  //F4
			Motor_mode(RIGHT_U);
			delay(delay_time*3);
			break;
		case 5:  //F5
			Motor_mode(RIGHT_U);
			delay(delay_time*3);
			break;
		case 6:  //F6
			Motor_mode(LEFT_U);
			delay(delay_time*2);
			break;
		case 7:  //R0
			Motor_mode(RIGHT_U);
			delay(delay_time);
			break;
		case 8:  //R1
			Motor_mode(RIGHT_U);
			delay(delay_time);
			break;
		case 9:  //R2
			break;
		case 10: //R3
			Motor_mode(LEFT_U);
			delay(delay_time);
			break;
		case 11: //R4
			Motor_mode(LEFT_U);
			delay(delay_time);
			break;
		}
		Motor_mode(STOP);
		delay(delay_time);

		PWM_value = temp_speed;  //forward�϶��� ���� ���ǵ��
		Motor_Control('A', PWM_value);

		ENCODER_CNT_L = 0;
		ENCODER_CNT_R = 0;  //���⼭���� pulse �� wheel rotation count ����(���� ���� ���ؼ� 0���� �ʱ�ȭ ����)

		Motor_mode(FORWARD);
		delay(delay_time*2);

		int random_num = random(1,3); //1���� 3���� ������ ���� �����Ͽ� ����
		switch(random_num){  //�����ϰ� ����Ʈī�� �����̱� ���� ��ġ (RIGHT_U�� �����̰ų� LEFT_U�� �����̰ų� �ƴϸ� FORWARD�θ� �����̰ų� �� �� �ϳ��� �����ϰ� �����δ�)
		case 1:
			Motor_mode(RIGHT_U);
			delay(delay_time);
			Motor_mode(FORWARD);
			delay(delay_time*2);
			break;
		case 2:
			Motor_mode(LEFT_U);
			delay(delay_time);
			Motor_mode(FORWARD);
			delay(delay_time*2);
			break;
		case 3:
			delay(delay_time*2);
			break;
		}

		Encoder_value_L += ENCODER_CNT_L;
		Encoder_value_R += ENCODER_CNT_R;  //���� pulse ���� �����ϴ� ������  wheel rotation count�� ������

		if(success%10 == 0 && success != 0){  // success �� 10�� ��� ����  "tricky mode" �۵� (������� ��� ����)
			int speed = PWM_value;
			PWM_value = 250;
			Motor_Control('A', PWM_value);
			Motor_mode(RIGHT_U);
			delay(delay_time*2);
			Motor_mode(LEFT_U);
			delay(delay_time*4);
			Motor_mode(RIGHT_U);
			delay(delay_time*2);
			PWM_value = speed;
			Motor_Control('A', PWM_value);
		}

		Motor_mode(STOP);
		Serial1.write(TX_buf0,5);
		flag=0;

		Timer2::start(); //����Ʈī�� �������� ���߾����Ƿ� LED Blink
	}
}

void serialEvent1(){
	int z, tmp=0;
	Serial1.readBytes(RX_buf, 17);
	if((RX_buf[0] == 0x76)&&(RX_buf[1]==0)) //���� ������ ���
	{
		for(z=2;z<16;z++)
			tmp += (unsigned char) RX_buf[z]; //ID�� �Էµ� ���� ��� ����
		tmp = tmp&0xFF;
		if((unsigned char) RX_buf[16] == tmp){ //���ؼ� ������
			for(z=0;z<12;z++)
			{
				RX_buf2[z] = (unsigned char)RX_buf[z+4]; //���Ĺ� ������ �� 12�� ���� ���� ����

				/*������ ������ ��(RX_before) �� (RX_buf2)�� ���� ����Ͽ��� ����ġ�� ���ϴ� ����1*/
				if(RX_before[z] == 0) // �ʱ���� => RX_before[z]=����, RX_buf2[z]=����
					RX_before[z] = RX_buf[z];
				if((RX_before[z] - RX_buf2[z])>0 ) //����ġ üũ, (���ο� ���� ���� ���� ������ ������ �۾Ơ��� �� = ���� ����� ���� ��)
					score[z] += 1; //�� ��� ������ score�� ���� 1 ������(������ ����ġ �ο���)
				if((RX_before[z] - RX_buf2[z])>0 && (RX_before[z] - RX_buf2[z])<=3 ) //���� ������ ����ġ üũ, (���ο� ���� ���� ���� ������ ������ 3 ���Ϸ� �۾����� ��)
					score[z] += 5; // �� ��� ������ ����ڰ� �ٰ����� �ִٰ� �Ǵ��ϰ�  score�� ���� 5 ������(���� ū ����ġ�� �ο��ϴ� ����)

				/*������� ���� �ӵ��� ���߾� ����Ʈī�� �ӵ��� ��ȭ�ֱ� ���� ����1*/
				sensor_temp[z] += RX_buf2[z];
			}

			/*������� ���� �ӵ��� ���߾� ����Ʈī�� �ӵ��� ��ȭ�ֱ� ���� ����2*/
			//������ ���� 4���� ������ ��� �����ִ� �����̴�. �����ִ� ������ ���� ����1���� �����Ѵ�.
			sensor_count++; //���� ���° sensor�� �����ִ� ������ count�� �Ͽ� Ȯ���Ѵ�

			if(sensor_count == 4) { //4��° sensor���� ������ ��,
				for(z=0;z<12;z++){
					sensor_sum1[z] = sensor_temp[z]; //sum1�� 4���� ���� ���� temp���� �޾ƿ� �����ϰ�
					sensor_temp[z] = 0; //temp�� �ʱ�ȭ
				}
			}
			if(sensor_count == 8){ //4��° ���� sum1�� ������ �� �� ���� 4����  sensor ���� ������ ��,
				for(z=0;z<12;z++){
					sensor_sum2[z] = sensor_temp[z]; //sum2�� 4���� ���� ���� temp���� �޾ƿ� �����ϰ�
					sensor_temp[z] = 0; //temp�� �ʱ�ȭ
				}
				sensor_count = 0;
			}
		}

		/*������ ������ ��(RX_before) �� (RX_buf2)�� ���� ����Ͽ��� ����ġ�� ���ϴ� ����2*/
		for(z=0;z<12;z++)
			RX_before[z] = RX_buf2[z]; // before�� buf2 ���� �־��ְ�(buf2�� ���� �޾ƿ� sensor���� ���� ���� ���ϱ� ���� ��������)
	}
	else{ // ���۹���Ʈ (76,0)�� �ƴϸ�
		for(z=1;z<17;z++){ //�� ���� ����Ʈ���� 76, 0 (���� ����Ʈ) �ٽ� ã��
			if(RX_buf[z] == 0x76){ //76 ã������
				if(z!=16){ //76�� ������ ����Ʈ�� �ƴ϶�� ���� ��Ʈ�� 0���� üũ
					//76&0�̶�� �ű⼭���� ���� �����̴ϱ� �� �ձ��� ��� �о ����
					if(RX_buf[z+1] == 0){
						tmp =z;
						break;
					}
				}else tmp = z; //z==16, �� 76�� ������ ����Ʈ��� 0~15���� 16���� ��� �о ����
				//76�� �����, �� ���� �ٽ� üũ�ϵ���
			}
		} Serial1.readBytes(RX_buf,tmp);//�о ����
	}

	/*����ġ�� ���ϴ� ����3 ����ġ�� �ο��ϸ� ���� score ���� �̿��ϴ� ����*/
	getMaxValueIndex(score, 12);
	//score �迭���� ���� ū sensor ���� �� sensor�� index�� �ҷ����� �Լ� (score�� ���� ũ�ٴ� ���� ����ġ�� ���� ũ�ٴ� ���̰�,�̴� ����ڰ� õõ�� ����Ʈī�� Ư�� ������ ���� �ٰ����� �ִٴ� �ǹ��̴�)

	if(score_max >= score_std_value)  // score�� ���� ������ �Ǵ� ���� score_std_value= 25 ��� ����. �̰ͺ��� ũ�� ����ڰ� �ٰ����� �ִٰ� �Ǵ�
	{
		if(RX_buf2[score_max_index] < RX_std_value) //������ �Ÿ��� RX_std_value=60���� ������ ����(���������ӿ��� ����ġ�� Ŀ���� ������ ���ܽ�Ű�� ����, ����ڰ� ������ �ִ��� �ѹ��� Ȯ���ϱ� ����)
		{
			/*�ڿ� ����ڸ� �ν��ϰ� ����Ʈī�� ������ ������ �������� �ʰ� �տ� ���� ���� ��ֹ��� �ִ��� �� �ѹ� Ȯ���ϱ� ���� ��ġ*/
			if(8<=score_max_index && score_max_index<=10){  //�Ĺ� ����(R1,R2,R3)���� ����ڰ� ������ ���
				getMinValueIndex(RX_buf2); //���� ������ �߿��� ���� ��ֹ��� ����� ������ ���ϱ� ���� �Լ�
				if((RX_min < 70) && (RX_min!=9)){
					//9�� ������ ���� �׻� ������ ������ ������ ���ܽ�Ű�� ����. 70���� ���������� 70cm���������������� �غ� ���ϸ� ���� �ε�ħ
					avoid_front_wall(); //���� ���ǵ��� ��� ������ ��� ���� ��ֹ�(��)�� ���� �� �ִ� �Լ��� �����Ŵ.
				}
			}

			controlSpeed(); //������� ���� �ӵ��� ���� ����Ʈī�� �ӵ��� ��ȭ�� �ִ� �Լ�
			RX_index = score_max_index; //loop�� switch������ ���Ǵ� RX_index�� ���� ����ڰ� �ٰ����� �ִٰ� ���̵� ������ index���� �־���
			score_max_index = 0;//���� �־��ְ� �ٽ� �ʱ�ȭ
			for(z=0;z<12;z++)
			{
				score[z] = 0;//score �迭�� �ʱ�ȭ
				RX_before[z] = 0;//RX_before �ʱ�ȭ
			}
			flag = 1; //loop�� �� �� �ְ� flag�� 1�� ����.
		}

		if(score_max>=80) // score�� ����ġ�� ������ ��츦 �����ϱ� ���ؼ�. ó������ �ٽ� ���� (����Ʈī�� ������ �ִ� ��쿡�� ����ġ�� ���ݾ� ���̱� ������ �׷��� ��� �ʱ�ȭ�� ������� ��)
		{
			for(z=0;z<12;z++)
			{
				score[z] = 0;//score �迭�� �ʱ�ȭ
				RX_before[z] = 0;//RX_before �ʱ�ȭ
				sensor_temp[z] = 0;
			}
			sensor_count = 0;
		}
	}
}

void serialEvent(){ //�ۿ��� ���� �޾ƿ��� ���� ������ �Լ�
	if(Serial.available()>0){ //���� �ۿ��� �޾ƿ� ���� ���� ���
		int command = Serial.read();
		int z;

		switch(command){
		case 1:  //ó�� start button�� ������ ������ ��.

			Serial.print(success); //success ���� ���
			start_signal = 1; //����� ��ġ ����(loop��) on

			Motor_mode(STOP);
			setBuzzer();

			Timer2::start();
			LED_state=1;
			LED_set(LED_state);

			Serial1.write(TX_buf0,5);
			break;

		case 2:  //����ڰ� stop ��ư�� ���� ��
			//���� �ӽ������� �����ϰ� success���� �������.
			//�ӽ������� ����Ʈī�� ���߰� ��.
			Serial1.write(TX_bufs,5);

			start_signal = 0; //����� ��ġ ����(loop��) off

			Motor_mode(STOP);
			Serial.print("stop");
			Timer2::stop();
			LED_state=0;
			LED_set(LED_state); //LED off

			for(z=0;z<12;z++)
			{
				score[z] = 0;//score �迭�� �ʱ�ȭ
				RX_before[z] = 0;//RX_before �ʱ�ȭ
				sensor_temp[z] = 0;
			}
			sensor_count = 0;

			break;

		case 3:  //����ڰ� stop ��ư�� ���� ���� ����� �ϴ� ���
			//������� �� ������ ó���� ����Ʈī�� ��ġ ������ �� ���� �����.
			//success���� ����ؾ� ��.
			Serial1.write(TX_buf0,5);
			Serial.print(success);
			Motor_mode(STOP);
			start_signal = 1; //����� ��ġ ���� (loop��) on

			Timer2::start();
			LED_state=1;
			LED_set(LED_state);
			break;

		case 4:  //����ڰ� stop�� longClick �� ���.
			//����Ʈī�� ������ ���߰� ��ȯ�ϴ� ���� ����. ��� ���� �ʱⰪ���� ����.
			Serial1.write(TX_bufs,5);

			Motor_mode(STOP);
			start_signal = 0; //����� ��ġ ����(loop��) off
			Serial.print("over");
			Timer2::stop();
			LED_state=0;
			LED_set(LED_state);

			for(z=0;z<12;z++)
			{
				score[z] = 0;//score �迭�� �ʱ�ȭ
				RX_before[z] = 0;//RX_before �ʱ�ȭ
				sensor_temp[z] = 0;
			}
			sensor_count = 0;

			PWM_value = 120;
			success=0;
			Encoder_value_L=0; Encoder_value_R=0;
			break;

		case 5:  //time left�� 0�� ��
			Serial.print(momentum_value); //������� ����� ������ ����

			start_signal = 0; //����� ��ġ ����(loop��) off
			Motor_mode(STOP);
			Timer2::stop();
			LED_state=0;
			LED_set(LED_state);
			Serial1.write(TX_bufs,5);

			for(z=0;z<12;z++)
			{
				score[z] = 0;//score �迭�� �ʱ�ȭ
				RX_before[z] = 0;//RX_before �ʱ�ȭ
				sensor_temp[z] = 0;
			}
			sensor_count = 0;

			PWM_value = 120;
			success=0;
			Encoder_value_R=0; Encoder_value_R=0; //distance
			break;

		case 6:  // smart car �� error �� ���� rescue button �� ���� ���
			start_signal = 0; //����� ��ġ ����(loop��) on

			Motor_mode(STOP);
			setBuzzer();

			Serial.print("rescue");

			Timer2::start();
			LED_state=1;
			LED_set(LED_state);

			Serial1.write(TX_buf0,5);

			Motor_mode(BACKWARD);
			delay(delay_time*5);
			break;

		default:
			break;
		}
	}
}

void getMaxValueIndex(unsigned long int score[], int size){ //����ġ�� �����ϴ� score���� ���� ū ���� ���� ���� index�� ���ϴ� �Լ�
	score_max = score[0];
	score_max_index = 0;
	for(int j=1; j<size ; j++){
		if(score[j] >= score_max){
			score_max = score[j];
			score_max_index = j;
		}
	}
}

void getMinValueIndex(unsigned long int RX_front[]){ // �տ��� �����Ǵ� ������ �ּ��� �Ÿ��� ��� ��ֹ��� ���ϱ� ���� �Լ� - �տ��� �����Ǵ� ������ �� ������ ���� ���� ���� ����, ���� index�� ���ϴ� �Լ�
	RX_min = RX_front[1];
	RX_min_index = 1;
	for(int j=2; j<=5 ; j++){
		if(RX_front[j] <= RX_min){
			RX_min = RX_front[j];
			RX_min_index = j;
		}
	}
}

void avoid_front_wall(){  // �ڿ��� ����ڰ� �ٰ����µ� �տ� ��ֹ��� ���� ��� �����Ű�� �Լ�
	int speed = PWM_value;
	PWM_value = 250;
	Motor_Control('A', PWM_value);
	Motor_mode(BACKWARD);
	delay(delay_time*3);
	switch(RX_min_index){
	case 1: //F1
		Motor_mode(RIGHT_U);
		delay(delay_time);
		break;
	case 2: //F2
		Motor_mode(RIGHT_U);
		delay(delay_time*2);
		break;
	case 3: //F3
		Motor_mode(RIGHT_U);
		delay(delay_time*2);
		break;
	case 4: //F4
		Motor_mode(LEFT_U);
		delay(delay_time*2);
		break;
	case 5: //F5
		Motor_mode(LEFT_U);
		delay(delay_time);
		break;
	}
	PWM_value = speed;
	Motor_Control('A', PWM_value);
}

void Encoder_count_L()
{
	ENCODER_CNT_L++;
}
void Encoder_count_R()
{
	ENCODER_CNT_R++;
}

void Timer2_ISR(){ //LED blink �ϱ� ���� Ÿ�̸�
	digitalWrite(Front_LED, LED_state);
	digitalWrite(Back_LED, LED_state);
	if(LED_state)
		LED_state=0;
	else
		LED_state=1;
}

void LED_set(int LED_state){ //LED ���¸� �ݿ��ϱ� ���� �Լ�
	digitalWrite(Front_LED, LED_state);
	digitalWrite(Back_LED, LED_state);
}

void controlSpeed(){ //���ǵ带 �����ϱ� ���� �Լ�
	int speed = sensor_sum1[score_max_index] - sensor_sum2[score_max_index];
	int speed_abs = abs(speed);
	if(speed_abs<15){
		if(PWM_value > 145)
			PWM_value -= 30;
		delay_time = 350;
	}
	else if(speed_abs<30){
		delay_time = 350;}
	else{
		if(PWM_value < 220)
			PWM_value += 30;
		delay_time = 300;}
}

void setBuzzer(){
	digitalWrite(BUZZER, 1);
	unsigned long time = millis();
	while(1){
		unsigned long time2 = millis();
		if( time2 > (time + 1000)){
			digitalWrite(BUZZER, 0);
			break;
		}
	}
}

void Motor_mode(int da){
	int z;
	for(z=0;z<4;z++)
		digitalWrite(Motor[z], (da>>z)&0x01 );
}

void Motor_Control(char da, unsigned int OC_value){
	switch(da){
	case 'L':
		analogWrite(Motor[4], OC_value);
		break;
	case 'R':
		analogWrite(Motor[5], OC_value);
	case 'A':
		analogWrite(Motor[4], OC_value);
		analogWrite(Motor[5], OC_value);
		break;
	}
}





