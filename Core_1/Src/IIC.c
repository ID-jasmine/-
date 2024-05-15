#include "IIC.h"
#include "delay.h"
#include "main.h"

/**
*	����˵�������ģ�� IIC
*
*	�������ܣ�GPIO ��ʼ��
*
*	ע�����GPIO ������ѡ��
*
*	��¼��	GPIO ���ų�ʼ��,��ͬ�Ĺ�
*			��GPIOX������ͬһ����У�
**/
void IIC_Init(void)
{		
//	GPIO_InitTypeDef GPIO_InitStructure;
//	
//	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOB, ENABLE );	//ʹ��GPIOBʱ��
//	   
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7;	//SCL & SDA
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD ;   	//����/��©
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;		//�ٶ� 10 MHz
//	GPIO_Init(GPIOB, &GPIO_InitStructure);
//	GPIO_SetBits(GPIOB,GPIO_Pin_6|GPIO_Pin_7); 				//PB6,PB7 �����
	
/*  �Ѿ���MX_GPIO_Init();�ж�����*/
	  
}


/**
*	����˵��������IIC��ʼ�ź�
*
*	ԭ��˵����	�� SCL �Ǹߵ�ƽ״̬ʱ,
*				������ SDA �ɸ�����;
**/
void IIC_Start(void)
{
    SDA_OUT();		/* SDA �����ģʽ */
	HAL_GPIO_WritePin(BH1750_SCL_GPIO_Port,BH1750_SCL_Pin,GPIO_PIN_RESET);
    HAL_GPIO_WritePin(BH1750_SDA_GPIO_Port,BH1750_SDA_Pin,GPIO_PIN_SET);	/* ������ SDA ״̬ */
    HAL_GPIO_WritePin(BH1750_SCL_GPIO_Port,BH1750_SCL_Pin,GPIO_PIN_SET);	/* ������ SCL ״̬ */
    IIC_Delay();	/* ��ʱ���ֵ�ƽ״̬ */
    HAL_GPIO_WritePin(BH1750_SDA_GPIO_Port,BH1750_SDA_Pin,GPIO_PIN_RESET);	/* ������ SDA ״̬ */
    IIC_Delay();	/* ��ʱ���ֵ�ƽ״̬ */
    HAL_GPIO_WritePin(BH1750_SCL_GPIO_Port,BH1750_SCL_Pin,GPIO_PIN_RESET);	/* ǯסI2C,���ͻ���� */
}



/**
*	����˵��������IICֹͣ�ź�
*
*	ԭ��˵����	SCL��Ϊ�ߵ�ƽ״̬ʱ, SDA ��
*				�ɵ͵�ƽ��ߵ�ƽ����(������)
**/
void IIC_Stop(void)
{
    SDA_OUT();		/* SDA �����ģʽ */
    HAL_GPIO_WritePin(BH1750_SCL_GPIO_Port,BH1750_SCL_Pin,GPIO_PIN_RESET);	/* ������ SCL ״̬ */
    HAL_GPIO_WritePin(BH1750_SDA_GPIO_Port,BH1750_SDA_Pin,GPIO_PIN_RESET);	/* ������ SDA ״̬ */
    IIC_Delay();	/* ��ʱ���ֵ�ƽ״̬ */
    HAL_GPIO_WritePin(BH1750_SCL_GPIO_Port,BH1750_SCL_Pin,GPIO_PIN_SET);	/* ������ SCL ״̬ */
    HAL_GPIO_WritePin(BH1750_SDA_GPIO_Port,BH1750_SDA_Pin,GPIO_PIN_SET);	/* ������ SDA ״̬ */
    IIC_Delay();	/* ��ʱ���ֵ�ƽ״̬ */
}



/**
*	����˵�����ȴ�Ӧ���źŵ���
*
*	ԭ��˵����	�Ƚ�����������,��ʱ�ȴ��ȶ�,Ȼ��ʱ��������,
*				��ʱ�ȴ��ȶ�,�����������ߵ�ƽ״̬,����Ǹ�
*				��ƽ,δӦ��,����ǵ͵�ƽ,Ӧ��;
*
*	����ֵ��0, ����Ӧ��ʧ��
*			1, ����Ӧ��ɹ�
**/
uint8_t IIC_Wait_Ack(void)
{
    uint16_t ErrorTime;
	
    SDA_IN();      			/** SDA ������ģʽ **/
    HAL_GPIO_WritePin(BH1750_SDA_GPIO_Port,BH1750_SDA_Pin,GPIO_PIN_SET);			/* ������ SDA ״̬ */
    IIC_Delay();			/* ��ʱ���ֵ�ƽ״̬ */
    HAL_GPIO_WritePin(BH1750_SCL_GPIO_Port,BH1750_SCL_Pin,GPIO_PIN_SET);			/* ������ SCL ״̬ */
    IIC_Delay();			/* ��ʱ���ֵ�ƽ״̬ */
	
//    while(HAL_GPIO_ReadPin(BH1750_SDA_GPIO_Port,BH1750_SDA_Pin))			/* ���� SDA ��ƽ״̬ */
	while(HAL_GPIO_ReadPin(BH1750_SDA_GPIO_Port,BH1750_SDA_Pin))			/* ���� SDA ��ƽ״̬ */
    {
        ErrorTime ++;
		
        if(ErrorTime > 250)
        {
            IIC_Stop();		/* ��Ӧ��,����ͨ�� */
            return 0;
        }
    }
    HAL_GPIO_WritePin(BH1750_SCL_GPIO_Port,BH1750_SCL_Pin,GPIO_PIN_RESET);			/** ǯס,�ȴ������� **/
	
    return 1;				/** ���յ�Ӧ���ź� **/
}


/**
*	����˵�������� ACK Ӧ��
*
*	ԭ��˵����SCL �ڸߵ�ƽ�ڼ� SDA ʼ�մ��ڵ͵�ƽ
*			 ��SCL ����ʱ�� <= SDA ����ʱ�䣩
*			  ��Ҫ�ڴ������һ���ֽں���
*
**/
void IIC_Ack(void)
{
	SDA_OUT();					/** SDA �����ģʽ **/
    HAL_GPIO_WritePin(BH1750_SCL_GPIO_Port,BH1750_SCL_Pin,GPIO_PIN_RESET);				/* ʱ���� SCL ���� */
    HAL_GPIO_WritePin(BH1750_SDA_GPIO_Port,BH1750_SDA_Pin,GPIO_PIN_RESET);				/* ������ SDA ���� */
    IIC_Delay();				/* ��ʱ���ֵ�ƽ״̬ */
    HAL_GPIO_WritePin(BH1750_SCL_GPIO_Port,BH1750_SCL_Pin,GPIO_PIN_SET);				/* ʱ���� SCL ���� */
    IIC_Delay();				/* ��ʱ���ֵ�ƽ״̬ */
    HAL_GPIO_WritePin(BH1750_SCL_GPIO_Port,BH1750_SCL_Pin,GPIO_PIN_RESET);				/* ʱ���� SCL ���� */
}


/**
*	����˵�������� NACK Ӧ��
*
*	ԭ��˵����SCL�ڸߵ�ƽ�ڼ�SDAʼ�մ��ڸߵ�ƽ
*			 ��SCL ����ʱ�� <= SDA ����ʱ�䣩
*			  ��Ҫ�ڴ������һ���ֽں���
*
**/
void IIC_NAck(void)
{
	SDA_OUT();					/** SDA �����ģʽ **/
    HAL_GPIO_WritePin(BH1750_SCL_GPIO_Port,BH1750_SCL_Pin,GPIO_PIN_RESET);				/* ������ SCL ���� */
    HAL_GPIO_WritePin(BH1750_SDA_GPIO_Port,BH1750_SDA_Pin,GPIO_PIN_SET);				/* ������ SDA ���� */
    IIC_Delay();				/* ��ʱ���ֵ�ƽ״̬ */
    HAL_GPIO_WritePin(BH1750_SCL_GPIO_Port,BH1750_SCL_Pin,GPIO_PIN_SET);				/* ʱ���� SCL ���� */
    IIC_Delay();				/* ��ʱ���ֵ�ƽ״̬ */
    HAL_GPIO_WritePin(BH1750_SCL_GPIO_Port,BH1750_SCL_Pin,GPIO_PIN_RESET);				/* ʱ���� SCL ���� */
}



/**
*	�������ܣ�IIC����һ���ֽ�
*	����˵����Txd	�޷��� 8 λ
*	
*	����ֵ��1, ��Ӧ��
*			0, ��Ӧ��
**/
void IIC_Send_Byte(uint8_t Txd)
{
    uint8_t T;
	
    SDA_OUT();						/** ������SDA���̬ **/
    HAL_GPIO_WritePin(BH1750_SCL_GPIO_Port,BH1750_SCL_Pin,GPIO_PIN_RESET);					/** ʱ���� SCL ���� **/
	
    for(T = 0; T < 8; T++)			/** �ɸߵ��ͷ������� */
    {
//        IIC_SDA = (Txd & 0x80) >> 7;/** ÿ�η������λ **/
		if((Txd & 0x80) >> 7)	HAL_GPIO_WritePin(BH1750_SDA_GPIO_Port,BH1750_SDA_Pin,GPIO_PIN_SET);
		else	HAL_GPIO_WritePin(BH1750_SDA_GPIO_Port,BH1750_SDA_Pin,GPIO_PIN_RESET);
        Txd <<= 1;					/* �������ݵ����λ */
        IIC_Delay();				/* ��ʱ���ֵ�ƽ״̬ */
        HAL_GPIO_WritePin(BH1750_SCL_GPIO_Port,BH1750_SCL_Pin,GPIO_PIN_SET);				/* ʱ���� SCL ���� */
        IIC_Delay();				/* ��ʱ���ֵ�ƽ״̬ */
        HAL_GPIO_WritePin(BH1750_SCL_GPIO_Port,BH1750_SCL_Pin,GPIO_PIN_RESET);				/* ʱ���� SCL ���� */
        IIC_Delay();				/* ��ʱ���ֵ�ƽ״̬ */
    }
}


/*
 * �������ܣ�IIC���ֽ�
 * ����˵����ack = 1, ����ACK
 *			 ack = 0, ����NACK
 *
 * ����ֵ�����ض�ȡ����һ���ֽ�
 */
uint8_t IIC_Read_Byte(uint8_t ACK)
{
	uint8_t	T;
	uint8_t	Receive;

    SDA_IN();				/** SDA ������Ϊ���� **/
	
    for(T = 0; T < 8; T++ )	/** �ɸߵ��ͽ�������**/
    {
        HAL_GPIO_WritePin(BH1750_SCL_GPIO_Port,BH1750_SCL_Pin,GPIO_PIN_RESET);		/* ʱ�������͵�ʱ��,SDA������仯 */
        IIC_Delay();		/* ��ʱ���ֵ�ƽ״̬ */
        HAL_GPIO_WritePin(BH1750_SCL_GPIO_Port,BH1750_SCL_Pin,GPIO_PIN_SET);		/* ����ʱ����,������SDA�仯,���Զ�ȡ SDA */
        Receive <<= 1;		/** ���յ�����λ���� **/
        if(HAL_GPIO_ReadPin(BH1750_SDA_GPIO_Port,BH1750_SDA_Pin))
        {
            Receive++;		/** �ߵ�ƽ,�����λΪ1 **/
        }
        IIC_Delay();		/* ��ʱ���ֵ�ƽ״̬ */
    }
	
    if (!ACK)	
		IIC_NAck();			/** ���� NACK **/
    else		
		IIC_Ack(); 			/** ����  ACK **/
    return Receive;
}
