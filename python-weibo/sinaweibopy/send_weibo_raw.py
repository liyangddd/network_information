#!/usr/bin/python  
#-*-coding:utf-8-*- 

import weibo
'''
APP_KEY = '2165372710'
APP_SECRET = 'd5a1e117967b8ca00da3a0667e6689c5'
CALL_BACK = 'http://127.0.0.1/callback.php'
'''
APP_KEY = '3551454856'
APP_SECRET = '4eccf634a2728734dcd6211c4fe4ad4a'
CALL_BACK = 'http://127.0.0.1/callback2.php'


def run():
        #weibo模块的APIClient是进行授权、API操作的类，先定义一个该类对象，传入参数为APP_KEY, APP_SECRET, CALL_BACK
	client = weibo.APIClient(APP_KEY, APP_SECRET, CALL_BACK)
        #获取该应用（APP_KEY是唯一的）提供给用户进行授权的url
	auth_url = client.get_authorize_url()
	#打印出用户进行授权的url，将该url拷贝到浏览器中，服务器将会返回一个url，该url中包含一个code字段（如图1所示）
	print "auth_url : " + auth_url
	#输入该code值（如图2所示）
	code = raw_input("input the retured code : ")
	#通过该code获取access_token，r是返回的授权结果，具体参数参考官方文档：
	# http://open.weibo.com/wiki/Oauth2/access_token
	r = client.request_access_token(code)
        #将access_token和expire_in设置到client对象
	client.set_access_token(r.access_token, r.expires_in)
 
	#print client.statuses.public_timeline.get()
	#以上步骤就是授权的过程，现在的client就可以随意调用接口进行微博操作了，下面的代码就是用用户输入的内容发一条新微博

	while True:
		print "Ready! Do you want to send a new weibo?(y/n)"
		choice = raw_input()
		if choice == 'y' or choice == 'Y':
			content = raw_input('input the your new weibo content : ')
			if content:
                                #调用接口发一条新微薄，status参数就是微博内容
				client.statuses.update.post(status=content)
				print "Send succesfully!"
				break;
			else:
				print "Error! Empty content!"
		if choice == 'n' or choice == 'N':
			break


if __name__ == "__main__":
	run()
