#include "tinyxml.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
// ���� g++ testxml.cpp -L./ -ltinyxml

int main()
{
    TiXmlDocument mydoc("./config.xml");//xml�ĵ�����  
    bool loadOk=mydoc.LoadFile();//�����ĵ�  
    if(!loadOk)  
    {  
        printf("could not load the xml file: %s\n",mydoc.ErrorDesc());  
        exit(1);  
    }  
  
    TiXmlElement *RootElement=mydoc.RootElement();  //��Ԫ��, Info  
    printf("[root name]  %s\n", RootElement->Value());  
      
    TiXmlElement *pEle=RootElement;  
  
    //�����ý��  
    for(TiXmlElement *mvpElement = pEle->FirstChildElement("mvp");//��һ����Ԫ��  
        mvpElement != NULL;  
        mvpElement = mvpElement->NextSiblingElement("mvp"))//��һ���ֵ�Ԫ��  
    {  
        // mvpElement->Value() �ڵ�����
        printf("%s  ", mvpElement->Value());  
        printf("ip:%s  ",mvpElement->Attribute("ip"));
        printf("port:%s  ",mvpElement->Attribute("port"));
        int port=0;
        mvpElement->QueryIntAttribute("port", &port);
        printf("port[int]:%d  ", port);
        printf("enabled:%s  ",mvpElement->Attribute("enabled"));
        printf("\n");

        // �����Ԫ��
        for(TiXmlElement *clientElement = mvpElement->FirstChildElement("client");
            clientElement != NULL;
            clientElement = clientElement->NextSiblingElement("client"))
        {
            // ����
            printf("  %s  ", clientElement->Value());
            printf("enabled:%s  ",clientElement->Attribute("enabled"));
            printf("\n");
            // ��Ԫ��
            printf("    %s:%s  ", clientElement->FirstChildElement("user")->Value(), clientElement->FirstChildElement("user")->GetText());
            printf("    %s:%s  ", clientElement->FirstChildElement("pass")->Value(), clientElement->FirstChildElement("pass")->GetText());
            printf("    %s:%s ", clientElement->FirstChildElement("maxWnd")->Value(), clientElement->FirstChildElement("maxWnd")->GetText());
            printf("\n");
        }
        
        //�����Ԫ�ص�ֵ
        /*for(TiXmlElement *sonElement=mvpElement->FirstChildElement();  
        sonElement;
        sonElement=sonElement->NextSiblingElement())  
        {  
            printf("%s", sonElement->FirstChild()->Value());
        }*/
    }  
	return 0;
}

