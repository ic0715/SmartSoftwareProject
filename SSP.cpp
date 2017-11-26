// Do not remove the include below
#include "SmartCar_01.h"
#include "Timer2.h"

/*초음파 센서로 정확한 측정을 하기 위한 변수들*/
char RX_buf[17];
unsigned long int RX_buf2[12]; // 전방 7개 + 후방 5개 따로 저장 배열
unsigned long int RX_before[12]={0,};
unsigned long int score[12]={0,}; //가중치 반영
unsigned char TX_buf0[5] = {0x76, 0x00, 0xF0, 0x00, 0xF0}; // All(전후방 모든 센서 사용)
unsigned char TX_bufs[5] = {0x76, 0x00, 0x0F, 0x00, 0x0F}; //STOP

/*사용자의 운동량을 측정하기 위해서 스마트카의 회전수를 측정하기 위한 변수들*/
unsigned long int ENCODER_CNT_L=0, ENCODER_CNT_R=0; //pulse 수  count를 위한 변수
unsigned long int Encoder_value_L=0, Encoder_value_R=0; //최종 pulse 수를 저장하는 변수
#define momentum_value ((double)((Encoder_value_R)/390)*(double)50)/100 //사용자의 운동량은 운동한 거리 개념으로 출력한다. 단위는 m

/*사용자의 접근 속도에 따라서 스마트카의 속도 변화를 주기 위한 변수들*/
unsigned int sensor_sum1[12] = {0,}; //
unsigned int sensor_sum2[12] = {0,};
unsigned int sensor_temp[12] = {0,}; //센서값 4개 term 단위로 저장하는 배열
int sensor_count = 0;

/*함수 안에서 쓰이는 변수들*/
unsigned int RX_min=0;
unsigned int RX_min_index=0;
unsigned int score_max = score[0];
int score_max_index = 0; // getMaxValueIndex에서 받은 score_max_index
int RX_index = 0; // switch문 안에 들어갈 센서 번호

/*pin 값 설정*/
int Front_LED = 10;
int Back_LED = 9;
int LED_state=0;
int BUZZER = 45;
int Motor[6] = {22, 23, 24, 25, 4, 5};

int flag = 0; // 센서 감지시 1로 하여 스마트카를 움직일 수 있는 루프에 들어가기 위한 변수
int start_signal = 0; //앱으로 부터 값을 받아와서 start 버튼을 눌렀을 때 스마트카를 움직일 수 있는 루프에 들어가기 위한 변수, Stop 을 누르면 0이 되어 루프안에 들어가지 못함.

int delay_time =300; //delay_time은 0.3초로 설정
int PWM_value = 120; //속도는 120 으로 초기화

int success=0; // 사용자가 게임을 시작했을 때 스마트카를 잡았을 때마다 증가하는 success 변수
unsigned int score_std_value= 25; //score 가중치 따질 때 기준 값을 25로 부여
unsigned long int RX_std_value = 60; // 가중치는 std_value 값이 되었고, 사용자가 충분히 가까이 있는지 초음파 센서를 체크하는 기준 값을 60으로 부여

//The setup function is called once at startup of the sketch
void setup()
{
	// Add your initialization code here
	int z;
	delay(1000);
	Serial1.begin(115200); // ATmega2560 - ATmega128 통신
	Serial.begin(115200); // ATmega2560 - Android Device 통신

	pinMode(Front_LED,OUTPUT);
	pinMode(Back_LED,OUTPUT);
	pinMode(BUZZER, OUTPUT);
	LED_set(LED_state);

	Timer2::set(1000000,Timer2_ISR);  // LED를 1초에 한번씩 깜빡이기 위한  Timer set
	Timer2::start();

	attachInterrupt(6,Encoder_count_L,RISING);
	attachInterrupt(7,Encoder_count_R,RISING);

	for(z=0;z<6;z++){
		pinMode(Motor[z], OUTPUT);
		digitalWrite(Motor[z], LOW);}

	Serial1.write(TX_bufs, 5); //초음파 센싱은  STOP 상태로 초기화
	Motor_Control('A', PWM_value);
	Motor_mode(STOP); //스마트카 동작 상태도  STOP으로 초기화
}

// The loop function is called in an endless loop
void loop()
{
	//Add your repeated code here
	if(flag==1 && start_signal==1){ // 초음파 센서가 사용자의 동작을 감지하고(flag=1)&&사용자가 start 버튼을 눌렀을 때(start_signal=1)
		success++; //스마트카를  움직이게 했으므로 successs value를 더해줌

		digitalWrite(BUZZER, 0);

		Timer2::stop();
		LED_state=1;
		LED_set(LED_state);  //LED 깜빡임 정지하고 항상 켜짐 상태로 바꿔줌.

		for(int z=0; z<12; z++)
		{ sensor_temp[z] = 0;
		sensor_sum1[z] = 0;
		sensor_sum2[z] = 0;
		}
		sensor_count = 0;   //temp 값, sensor_count 값 초기화

		int temp_speed = PWM_value;  // 회전할 때는 full speed를 주기 위해서 임시로 원래 스피드를 저장하는 변수 temp_speed
		PWM_value = 250; // 회전할 때는 speed 250.
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

		PWM_value = temp_speed;  //forward일때는 원래 스피드로
		Motor_Control('A', PWM_value);

		ENCODER_CNT_L = 0;
		ENCODER_CNT_R = 0;  //여기서부터 pulse 로 wheel rotation count 시작(새로 세기 위해서 0으로 초기화 해줌)

		Motor_mode(FORWARD);
		delay(delay_time*2);

		int random_num = random(1,3); //1부터 3까지 랜덤한 수를 선택하여 저장
		switch(random_num){  //랜덤하게 스마트카를 움직이기 위한 장치 (RIGHT_U로 움직이거나 LEFT_U로 움직이거나 아니면 FORWARD로만 움직이거나 셋 중 하나로 랜덤하게 움직인다)
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
		Encoder_value_R += ENCODER_CNT_R;  //최종 pulse 수를 저장하는 변수에  wheel rotation count를 더해줌

		if(success%10 == 0 && success != 0){  // success 값 10의 배수 마다  "tricky mode" 작동 (사용자의 흥미 유발)
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

		Timer2::start(); //스마트카가 움직임을 멈추었으므로 LED Blink
	}
}

void serialEvent1(){
	int z, tmp=0;
	Serial1.readBytes(RX_buf, 17);
	if((RX_buf[0] == 0x76)&&(RX_buf[1]==0)) //센싱 시작인 경우
	{
		for(z=2;z<16;z++)
			tmp += (unsigned char) RX_buf[z]; //ID와 입력된 값을 모두 더함
		tmp = tmp&0xFF;
		if((unsigned char) RX_buf[16] == tmp){ //비교해서 같으면
			for(z=0;z<12;z++)
			{
				RX_buf2[z] = (unsigned char)RX_buf[z+4]; //전후방 초음파 값 12개 값만 따로 저장

				/*초음파 센싱의 전(RX_before) 후 (RX_buf2)의 차를 계산하여서 가중치를 구하는 과정1*/
				if(RX_before[z] == 0) // 초기상태 => RX_before[z]=이전, RX_buf2[z]=이후
					RX_before[z] = RX_buf[z];
				if((RX_before[z] - RX_buf2[z])>0 ) //가중치 체크, (새로운 센싱 값이 이전 센싱한 값보다 작아졋을 때 = 더욱 가까워 졌을 때)
					score[z] += 1; //이 경우 무조건 score의 값을 1 더해줌(조금의 가중치 부여함)
				if((RX_before[z] - RX_buf2[z])>0 && (RX_before[z] - RX_buf2[z])<=3 ) //더욱 세밀한 가중치 체크, (새로운 센싱 값이 이전 센상한 값보다 3 이하로 작아졌을 때)
					score[z] += 5; // 이 경우 실제로 사용자가 다가오고 있다고 판단하고  score의 값을 5 더해줌(더욱 큰 가중치를 부여하는 것임)

				/*사용자의 접근 속도에 맞추어 스마트카의 속도를 변화주기 위한 과정1*/
				sensor_temp[z] += RX_buf2[z];
			}

			/*사용자의 접근 속도에 맞추어 스마트카의 속도를 변화주기 위한 과정2*/
			//센싱한 값을 4개의 단위로 끊어서 더해주는 과정이다. 더해주는 과정은 위에 과정1에서 진행한다.
			sensor_count++; //현재 몇번째 sensor를 더해주는 중인지 count를 하여 확인한다

			if(sensor_count == 4) { //4번째 sensor까지 더했을 때,
				for(z=0;z<12;z++){
					sensor_sum1[z] = sensor_temp[z]; //sum1에 4개씩 더한 값을 temp에서 받아와 저장하고
					sensor_temp[z] = 0; //temp는 초기화
				}
			}
			if(sensor_count == 8){ //4번째 값을 sum1에 저장한 뒤 그 다음 4개의  sensor 값을 더했을 때,
				for(z=0;z<12;z++){
					sensor_sum2[z] = sensor_temp[z]; //sum2에 4개씩 더한 값을 temp에서 받아와 저장하고
					sensor_temp[z] = 0; //temp는 초기화
				}
				sensor_count = 0;
			}
		}

		/*초음파 센싱의 전(RX_before) 후 (RX_buf2)의 차를 계산하여서 가중치를 구하는 과정2*/
		for(z=0;z<12;z++)
			RX_before[z] = RX_buf2[z]; // before에 buf2 값을 넣어주고(buf2에 새로 받아올 sensor값과 현재 값을 비교하기 위해 저장해줌)
	}
	else{ // 시작바이트 (76,0)가 아니면
		for(z=1;z<17;z++){ //그 다음 바이트부터 76, 0 (시작 바이트) 다시 찾기
			if(RX_buf[z] == 0x76){ //76 찾았으면
				if(z!=16){ //76이 마지막 바이트가 아니라면 다음 비트가 0인지 체크
					//76&0이라면 거기서부터 새로 시작이니까 그 앞까지 모두 읽어서 버림
					if(RX_buf[z+1] == 0){
						tmp =z;
						break;
					}
				}else tmp = z; //z==16, 즉 76이 마지막 바이트라면 0~15까지 16개를 모두 읽어서 버림
				//76만 남기고, 그 다음 다시 체크하도록
			}
		} Serial1.readBytes(RX_buf,tmp);//읽어서 버림
	}

	/*가중치를 구하는 과정3 가중치를 부여하며 구한 score 값을 이용하는 과정*/
	getMaxValueIndex(score, 12);
	//score 배열에서 가장 큰 sensor 값과 그 sensor의 index를 불러오는 함수 (score이 가장 크다는 것은 가중치가 가장 크다는 것이고,이는 사용자가 천천히 스마트카의 특정 센서를 향해 다가가고 있다는 의미이다)

	if(score_max >= score_std_value)  // score의 가장 기준이 되는 값을 score_std_value= 25 라고 설정. 이것보다 크면 사용자가 다가오고 있다고 판단
	{
		if(RX_buf2[score_max_index] < RX_std_value) //센서의 거리가 RX_std_value=60보다 작은지 측정(정지상태임에도 가중치만 커지는 센서는 제외시키기 위함, 사용자가 가까이 있는지 한번더 확인하기 위함)
		{
			/*뒤에 사용자를 인식하고 스마트카가 무작정 앞으로 움직이지 않고 앞에 벽과 같은 장애물이 있는지 또 한번 확인하기 위한 장치*/
			if(8<=score_max_index && score_max_index<=10){  //후방 센서(R1,R2,R3)에서 사용자가 감지될 경우
				getMinValueIndex(RX_buf2); //앞의 센서들 중에서 가장 장애물과 가까운 센서를 구하기 위한 함수
				if((RX_min < 70) && (RX_min!=9)){
					//9를 제외한 것은 항상 센서에 잡히는 에러값 제외시키기 위함. 70으로 정한이유는 70cm떨어져있을때부터 준비를 안하면 벽에 부딪침
					avoid_front_wall(); //위의 조건들을 모두 만족한 경우 앞의 장애물(벽)을 피할 수 있는 함수를 실행시킴.
				}
			}

			controlSpeed(); //사용자의 접근 속도에 따라 스마트카의 속도에 변화를 주는 함수
			RX_index = score_max_index; //loop의 switch문에서 사용되는 RX_index에 현재 사용자가 다가오고 있다고 센싱된 센서의 index값을 넣어줌
			score_max_index = 0;//값을 넣어주고 다시 초기화
			for(z=0;z<12;z++)
			{
				score[z] = 0;//score 배열값 초기화
				RX_before[z] = 0;//RX_before 초기화
			}
			flag = 1; //loop에 들어갈 수 있게 flag를 1로 해줌.
		}

		if(score_max>=80) // score의 가중치가 누적될 경우를 방지하기 위해서. 처음부터 다시 시작 (스마트카가 가만히 있는 경우에도 가중치가 조금씩 쌓이기 때문에 그러한 경우 초기화를 시켜줘야 함)
		{
			for(z=0;z<12;z++)
			{
				score[z] = 0;//score 배열값 초기화
				RX_before[z] = 0;//RX_before 초기화
				sensor_temp[z] = 0;
			}
			sensor_count = 0;
		}
	}
}

void serialEvent(){ //앱에서 값을 받아오고 값을 보내는 함수
	if(Serial.available()>0){ //현재 앱에서 받아온 값이 있을 경우
		int command = Serial.read();
		int z;

		switch(command){
		case 1:  //처음 start button을 누르고 시작할 때.

			Serial.print(success); //success 값을 출력
			start_signal = 1; //사용자 위치 감지(loop문) on

			Motor_mode(STOP);
			setBuzzer();

			Timer2::start();
			LED_state=1;
			LED_set(LED_state);

			Serial1.write(TX_buf0,5);
			break;

		case 2:  //사용자가 stop 버튼을 누를 때
			//값을 임시적으로 저장하고 success수를 출력해줌.
			//임시적으로 스마트카를 멈추게 함.
			Serial1.write(TX_bufs,5);

			start_signal = 0; //사용자 위치 감지(loop문) off

			Motor_mode(STOP);
			Serial.print("stop");
			Timer2::stop();
			LED_state=0;
			LED_set(LED_state); //LED off

			for(z=0;z<12;z++)
			{
				score[z] = 0;//score 배열값 초기화
				RX_before[z] = 0;//RX_before 초기화
				sensor_temp[z] = 0;
			}
			sensor_count = 0;

			break;

		case 3:  //사용자가 stop 버튼을 누른 다음 재시작 하는 경우
			//재시작을 할 때에는 처음에 스마트카가 위치 재정비 후 게임 재시작.
			//success값을 출력해야 함.
			Serial1.write(TX_buf0,5);
			Serial.print(success);
			Motor_mode(STOP);
			start_signal = 1; //사용자 위치 감지 (loop문) on

			Timer2::start();
			LED_state=1;
			LED_set(LED_state);
			break;

		case 4:  //사용자가 stop을 longClick 한 경우.
			//스마트카가 완전히 멈추고 반환하는 값도 없음. 모든 값을 초기값으로 설정.
			Serial1.write(TX_bufs,5);

			Motor_mode(STOP);
			start_signal = 0; //사용자 위치 감지(loop문) off
			Serial.print("over");
			Timer2::stop();
			LED_state=0;
			LED_set(LED_state);

			for(z=0;z<12;z++)
			{
				score[z] = 0;//score 배열값 초기화
				RX_before[z] = 0;//RX_before 초기화
				sensor_temp[z] = 0;
			}
			sensor_count = 0;

			PWM_value = 120;
			success=0;
			Encoder_value_L=0; Encoder_value_R=0;
			break;

		case 5:  //time left가 0일 때
			Serial.print(momentum_value); //사용자의 운동량을 앱으로 전송

			start_signal = 0; //사용자 위치 감지(loop문) off
			Motor_mode(STOP);
			Timer2::stop();
			LED_state=0;
			LED_set(LED_state);
			Serial1.write(TX_bufs,5);

			for(z=0;z<12;z++)
			{
				score[z] = 0;//score 배열값 초기화
				RX_before[z] = 0;//RX_before 초기화
				sensor_temp[z] = 0;
			}
			sensor_count = 0;

			PWM_value = 120;
			success=0;
			Encoder_value_R=0; Encoder_value_R=0; //distance
			break;

		case 6:  // smart car 가 error 가 나서 rescue button 을 누를 경우
			start_signal = 0; //사용자 위치 감지(loop문) on

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

void getMaxValueIndex(unsigned long int score[], int size){ //가중치를 저장하는 score에서 가장 큰 센서 값과 센서 index를 구하는 함수
	score_max = score[0];
	score_max_index = 0;
	for(int j=1; j<size ; j++){
		if(score[j] >= score_max){
			score_max = score[j];
			score_max_index = j;
		}
	}
}

void getMinValueIndex(unsigned long int RX_front[]){ // 앞에서 감지되는 센서가 최소인 거리인 경우 장애물을 피하기 위한 함수 - 앞에서 감지되는 센서들 중 센싱한 값이 가장 작은 값과, 센서 index를 구하는 함수
	RX_min = RX_front[1];
	RX_min_index = 1;
	for(int j=2; j<=5 ; j++){
		if(RX_front[j] <= RX_min){
			RX_min = RX_front[j];
			RX_min_index = j;
		}
	}
}

void avoid_front_wall(){  // 뒤에서 사용자가 다가오는데 앞에 장애물이 있을 경우 실행시키는 함수
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

void Timer2_ISR(){ //LED blink 하기 위한 타이머
	digitalWrite(Front_LED, LED_state);
	digitalWrite(Back_LED, LED_state);
	if(LED_state)
		LED_state=0;
	else
		LED_state=1;
}

void LED_set(int LED_state){ //LED 상태를 반영하기 위한 함수
	digitalWrite(Front_LED, LED_state);
	digitalWrite(Back_LED, LED_state);
}

void controlSpeed(){ //스피드를 조절하기 위한 함수
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





