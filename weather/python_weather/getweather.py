#!/usr/bin/env python
# weather html parser
from HTMLParser import HTMLParser
import sys,urllib2,string,re
# define a class to parser a html
class HtmlParser(HTMLParser):
    def __init__(self):
        self.data=''
        self.readingdata=0
        HTMLParser.__init__(self)
    def handle_starttag(self,tag,attrs):
        if tag == 'td':
            self.readingdata=1
    def handle_data(self,chars):
        if self.readingdata:
            self.data+=chars
    def handle_endtag(self,tag):
        if tag=='td':
            self.readingdata=0
    def cleanse(self):
        self.data = re.sub('\s+',' ', self.data)
    def getdata(self):
        self.cleanse()
        return self.data
# this url is a place where you want to know the weather forecast
url="http://www.weather.com.cn/html/weather/101210501.shtml"
req=urllib2.Request(url)
fd=urllib2.urlopen(req)
tp=HtmlParser()
tp.feed(fd.read())
weather=tp.getdata()
# when you are getting a weather after parsering
# this weather string have 7 days weather forecast
# the following if for my awesome format
weather=weather.split()
tag=[weather.index(i) for i in weather if '\xe6\x97\xa5' in i]
first=weather[:tag[1]]
second=weather[tag[1]:tag[2]]
if second[1]!=second[7]:second[1]+=', '+second[7]
second[2]='low temp: '+second[9]+', high temp:'+second[3]
second[0]=second[0][:-6]
second=second[:3]
third=weather[tag[2]:tag[3]]
if third[1]!=third[7]:third[1]+=', '+third[7]
third[2]='low temp: '+third[9]+', high temp:'+third[3]
third[0]=third[0][:-6]
third=third[:3]
weather=['Weather:\n']+first+['\n']+second+['\n']+third
for i in weather:print i,
