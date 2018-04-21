#include "tinyxml.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
// 编译 g++ testxml.cpp -L./ -ltinyxml

int main()
{
    TiXmlDocument mydoc("./config.xml");//xml文档对象  
    bool loadOk=mydoc.LoadFile();//加载文档  
    if(!loadOk)  
    {  
        printf("could not load the xml file: %s\n",mydoc.ErrorDesc());  
        exit(1);  
    }  
  
    TiXmlElement *RootElement=mydoc.RootElement();  //根元素, Info  
    printf("[root name]  %s\n", RootElement->Value());  
      
    TiXmlElement *pEle=RootElement;  
  
    //遍历该结点  
    for(TiXmlElement *mvpElement = pEle->FirstChildElement("mvp");//第一个子元素  
        mvpElement != NULL;  
        mvpElement = mvpElement->NextSiblingElement("mvp"))//下一个兄弟元素  
    {  
        // mvpElement->Value() 节点名称
        printf("%s  ", mvpElement->Value());  
        printf("ip:%s  ",mvpElement->Attribute("ip"));
        printf("port:%s  ",mvpElement->Attribute("port"));
        int port=0;
        mvpElement->QueryIntAttribute("port", &port);
        printf("port[int]:%d  ", port);
        printf("enabled:%s  ",mvpElement->Attribute("enabled"));
        printf("\n");

        // 输出子元素
        for(TiXmlElement *clientElement = mvpElement->FirstChildElement("client");
            clientElement != NULL;
            clientElement = clientElement->NextSiblingElement("client"))
        {
            // 属性
            printf("  %s  ", clientElement->Value());
            printf("enabled:%s  ",clientElement->Attribute("enabled"));
            printf("\n");
            // 子元素
            printf("    %s:%s  ", clientElement->FirstChildElement("user")->Value(), clientElement->FirstChildElement("user")->GetText());
            printf("    %s:%s  ", clientElement->FirstChildElement("pass")->Value(), clientElement->FirstChildElement("pass")->GetText());
            printf("    %s:%s ", clientElement->FirstChildElement("maxWnd")->Value(), clientElement->FirstChildElement("maxWnd")->GetText());
            printf("\n");
        }
        
        //输出子元素的值
        /*for(TiXmlElement *sonElement=mvpElement->FirstChildElement();  
        sonElement;
        sonElement=sonElement->NextSiblingElement())  
        {  
            printf("%s", sonElement->FirstChild()->Value());
        }*/
    }  
	return 0;
}

