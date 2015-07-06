#!/usr/bin/env python
#-*-coding:utf-8-*-
 
import sys
import initclient
import json
import weibo

APP_KEY = '2165372710'
APP_SECRET = 'd5a1e117967b8ca00da3a0667e6689c5'
CALL_BACK = 'http://127.0.0.1/callback.php'

'''
APP_KEY = '3551454856 '
APP_SECRET = '4eccf634a2728734dcd6211c4fe4ad4a'
CALL_BACK = 'http://127.0.0.1/callback.php'
'''
def run():
        #调用initclietn模块创建授权的client对象
	client = initclient.get_client(APP_KEY, APP_SECRET, CALL_BACK)
	if not client:
                return
	args = sys.argv
	screen_name_input = args[1]
	#print "Input nickname of the microblog: ",
	#screen_name_input = raw_input()
	pf = client.users.show.get(screen_name = screen_name_input)
	print "---------------------------------------------------"
	print "Nickname: ", pf["name"],
	print "   ID: ", pf["id"]

        
	#当返回的json中status属性不存在时，说明该用户没有最新的微博,输出"This user do not have microblog content."
	have_weibo = hasattr(pf, "status")
	if have_weibo:
		print "{0:15}".format("Content:"), pf["status"]["text"]
        #由于当获取的微博为转发微博时，users.show.get()接口获取的微博信息无法获取转发的原来微博，因此通过该接口获取的转发的微博ID，然后通过statuses.show.get()接口获取该转发微博的原始微博信息
		get_by_weiboID = client.statuses.show.get(id = pf["status"]["id"])
		is_retweeted_status = hasattr(get_by_weiboID, "retweeted_status")
		if is_retweeted_status:
			print "Original Content:", get_by_weiboID["retweeted_status"]["text"]
		obj = pf["status"]
		have_picture = hasattr(obj, "original_pic")
		if have_picture:
			print "{0:15}".format("Picture:"), pf["status"]["original_pic"]
		print "{0:15}".format("Created time:"), pf["status"]["created_at"]
		#由于users.show.get端口获取的微博信息中获取的转发数、评论数均为0，因此使用该接口获得的微博id从statuses.show.get接口中获取转发数和评论数并输出
		#print "Repost: ", pf["status"]["reposts_count"]
		#print "   Comment: ", pf["status"]["comments_count"]
		weiboID = pf["status"]["id"]
		pfID = client.statuses.show.get(id = weiboID);
		print "Repost:", pfID["reposts_count"],
		print "      Comment:", pfID["comments_count"]
		print "---------------------------------------------------"
	else:
		print "This user do not have microblog content."
'''
	print "Input nickname of the microblog: ", 
	screen_name_input = raw_input()
	pf = client.statuses.user_timeline.get(screen_name = screen_name_input)
	
	print "---------------------------------------------------"
	print "Content: " + pf["statuses"][0]["text"]
	obj = pf["statuses"][0]
	have_picture = hasattr(obj, "bmiddle_pic")
	if have_picture:
	   print "Picture: ", pf["statuses"][0]["bmiddle_pic"]
	print "Created time: ", pf["statuses"][0]["created_at"]
	print "Repost: ", pf["statuses"][0]["reposts_count"],
	print " Comment: ", pf["statuses"][0]["comments_count"]
	print "---------------------------------------------------"
'''
'''
        #根据用户输入内容发微博
	while True:
		print "Ready! Do you want to send a new weibo?(y/n)"
		choice = raw_input()
		if choice == 'y' or choice == 'Y':
			content = raw_input('input the your new weibo content : ')
			if content:
				client.statuses.update.post(status=content)
				print "Send succesfully!"
				break;
			else:
				print "Error! Empty content!"
		if choice == 'n' or choice == 'N':
			break
'''
if __name__ == "__main__":
	run()
