#include <Arduino.h>
#include <Dhcp.h>
#include <Dns.h>
#include <EthernetServer.h>
#include <util.h>
#include <EthernetUdp.h>
#include <Ethernet.h>
#include <EthernetClient.h>
#include <SPI.h>
#include <stdlib.h>

#include "Nimbits.h"

int PORT = 80;
const char *GOOGLE = "google.com";

const int WAIT_TIME = 1000;

Nimbits::Nimbits(String instance, String ownerEmail, String accessKey)
{
  _instance = instance;
  _ownerEmail = ownerEmail;
  _accessKey = accessKey;
}

void Nimbits::createPoint(String pointName)
{
  EthernetClient client;
  if (client.connect(GOOGLE, PORT))
  {
    client.println(F("POST /service/point HTTP/1.1"));
    String content;
    //  writeAuthParams(content);
    content += "email=";
    content += _ownerEmail;
    if (_accessKey.length() > 0)
    {
      content += ("&key=");
      content += (_accessKey);
    }
    content += "&action=create&point=";
    content += (pointName);
    client.println(F("Host:nimbits1.appspot.com"));
    client.println(F("Connection:close"));
    client.println(F("Cache-Control:max-age=0"));
    client.print(F("Content-Type: application/x-www-form-urlencoded\n"));
    client.print(F("Content-Length: "));
    client.print(content.length());
    client.print(F("\n\n"));
    client.print(content);
    while (client.connected() && !client.available())
    {
      delay(1); //waits for data
    }
    client.stop();
    client.flush();
  }

}

String Nimbits::recordValue(String point, float value)
{
  EthernetClient client;
  String content;

  if (client.connect(GOOGLE, PORT))
  {
    client.println(F("POST /service/value HTTP/1.1"));
    //  writeAuthParams(content);
    content += ("email=");
    content += _ownerEmail;
    if (_accessKey.length() > 0)
    {
      content += ("&key=");
      content += (_accessKey);
    }

    char buffer[10];

    dtostrf(value, 5, 5, buffer);
    String str = buffer;

    content += ("&value=");
    content += (str);
    content += ("&point=");
    content += (point);
    client.println(F("Host:nimbits1.appspot.com"));
    client.println(F("Connection:close"));
    client.println(F("Cache-Control:max-age=0"));
    client.print(F("Content-Type: application/x-www-form-urlencoded\n"));
    client.print(F("Content-Length: "));
    client.print(content.length());
    client.print(F("\n\n"));
    client.print(content);
    while (client.connected() && !client.available())
    {
      delay(1); //waits for data
    }
    client.stop();
    client.flush();
  }

  return content;
}

long Nimbits::getTime()
{
  EthernetClient client;

  if (client.connect(GOOGLE, PORT))
  {
    client.print(F("GET /service/time?"));
    writeAuthParamsToClient(client);
    writeHostToClient(client);

    String response = getResponse(client);
    return atol(&response[0]);

  }
  else
  {
    return -1;
  }
}

float Nimbits::getValue(String pointName)
{
  EthernetClient client;

  if (client.connect(GOOGLE, PORT))
  {
    client.print(F("GET /service/value?"));
    writeAuthParamsToClient(client);
    client.print(F("&point="));
    client.print(pointName);
    writeHostToClient(client);

    return atof(&getResponse(client)[0]);
  }
  else
  {
    return -1;
  }

}

String Nimbits::getResponse(EthernetClient client)
{
  String result;
  boolean inData = false;
  char c;

  while (client.connected() && !client.available())
  {
    delay(1);
  }

  while (client.available())
  {
    c = client.read();
    if (c == '|')
    {
      inData = true;
    }
    else
    {
      if (inData && c != '|')
      {
        result += c;
      }
      else
      {
        if (inData && c == '|')
        {
          client.stop();
        }
      }
    }
  }

  return result;
}

void Nimbits::writeHostToClient(EthernetClient client)
{
  client.print(F("&client=arduino"));
  client.print(F(" "));
  client.println(F("HTTP/1.1"));
  client.print(F("Host:"));
  client.print(_instance);
  client.println(F(".appspot.com"));
  client.println();
}

void Nimbits::writeAuthParamsToClient(EthernetClient client)
{
  client.print(F("email="));
  client.print(_ownerEmail);
  if (_accessKey.length() > 0)
  {
    client.print(F("&key="));
    client.print(_accessKey);
  }
}

void Nimbits::writeAuthParams(String content)
{
  content += ("email=");
  content += (_ownerEmail);
  if (_accessKey.length() > 0)
  {
    content += ("&key=");
    content += (_accessKey);
  }
}

//record a value

//record data

//create point

//create point with parent

//batch update

//delete point

//get children with values

