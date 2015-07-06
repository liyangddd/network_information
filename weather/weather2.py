# -*- coding:utf-8 -*-

import sys

class Weather(object):
    def __init__(self, data):
        data = data['weatherinfo']
        self.city = data['city']
        self.id = data['cityid']
        self.date = ' '.join([data['date_y'], data['week']])
        t1 = ' '.join([data['temp1'], data['weather1']])
        t2 = ' '.join([data['temp2'], data['weather2']])
        t3 = ' '.join([data['temp3'], data['weather3']])
        t4 = ' '.join([data['temp4'], data['weather4']])
        t5 = ' '.join([data['temp5'], data['weather5']])
        t6 = ' '.join([data['temp6'], data['weather6']])
        self.weather = [t1, t2, t3, t4, t5, t6]
        self.suggest = data['index_d']
        self.original = data
    def report(self):
        days = [u'今天', u'明天', u'后天']
        print ' '
        print self.city, self.date
        print '-'*26
        for i in range(3):
            print days[i] + u'：' + self.weather[i]
        print '-'*26
        print self.suggest

def get_cityid(city = None):
    import urllib2
    if city == None:
        id_url = 'http://61.4.185.48:81/g/'
        id_data = urllib2.urlopen(id_url)
        city_id = id_data.readline().split(';')[1].split('=')[1]
    else:
        import codecs
        idfile = open('city_id.txt', 'r')
        for line in idfile:
            line = line.decode('gbk').encode('utf-8')
            if city in line:
                city_id = line.split(',')[0]
                break
            else:
                city_id = None
        if city_id == None:
            raise ValueError('City Name Incorrect')
    return city_id

def get_weather(city_id):
    import json
    import urllib2
    ids = str(city_id)
    base_url = 'http://m.weather.com.cn/data/'
    city_url = base_url + ids + '.html'
    try:
        data_return = urllib2.urlopen(city_url)
    except urllib2.URLError:
        raise ValueError('City ID Not Correct')
    weather_data = [i for i in data_return]
    data = json.loads(weather_data[0])
    output = Weather(data)
    return output

def Report(city='LOCAL'):
    if city == 'LOCAL':
        city_id = get_cityid()
        weather_data = get_weather(city_id)
        weather_data.report()
    else:
        city = city.decode('gbk').encode('utf-8')
        city_id = get_cityid(city)
        weather_data = get_weather(city_id)
        weather_data.report()

if __name__ == '__main__':
    if len(sys.argv) == 1:
        import time
        Report()
        time.sleep(5)
        sys.exit(0)
    else:
        for city in sys.argv[1:]:
            Report(city)


