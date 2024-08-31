from app.views.ViewsBase import *
from app.models import *
from django.shortcuts import render, redirect
from django.contrib.auth.models import User
from app.utils.Common import buildPageLabels


def web_alarms(request):
    context = {
    }
    data = []

    params = parse_get_params(request)

    page = params.get('p', 1)
    page_size = params.get('ps', 10)
    try:
        page = int(page)
    except:
        page = 1

    try:
        page_size = int(page_size)
        if page_size > 20 or page_size < 10:
            page_size = 10
    except:
        page_size = 10

    skip = (page - 1) * page_size
    sql_data = "select * from av_alarm order by id desc limit %d,%d " % (
        skip, page_size)
    sql_data_num = "select count(id) as count from av_alarm "

    count = g_djangoSql.select(sql_data_num)
    unread_count = 0
    if len(count) > 0:
        count = int(count[0]["count"])
        __data = g_djangoSql.select(sql_data)
        for i in __data:
            data.append({
                # "imageUrl":"http://127.0.0.1:9001/static/images/media.jpg",
                # "videoUrl":"http://127.0.0.1:9001/static/alarms/c4bb4965648175-1697194397420.mp4",
                "id": i["id"],
                "imageUrl": g_config.uploadDir_www + "/" + i["image_path"],
                "videoUrl": g_config.uploadDir_www + "/" + i["video_path"],
                "desc": i["desc"],
                "create_time": i["create_time"],
                "state": i["state"]
            })
        unread_count = g_djangoSql.select("select count(id) as count from av_alarm where state=0")
        if len(unread_count) > 0:
            unread_count = int(unread_count[0]["count"])
        else:
            unread_count = 0
    else:
        count = 0

    page_num = int(count / page_size)  # 总页数
    if count % page_size > 0:
        page_num += 1
    pageLabels = buildPageLabels(page=page, page_num=page_num)
    pageData = {
        "page": page,
        "page_size": page_size,
        "page_num": page_num,
        "count": count,
        "pageLabels": pageLabels
    }

    context["data"] = data
    context["pageData"] = pageData
    top_msg = ""
    if unread_count > 0:
        top_msg = "未读报警数据%d条" % unread_count

    context["top_msg"] = top_msg

    return render(request, 'app/web_alarms.html', context)
