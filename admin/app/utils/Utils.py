import uuid
import random
import datetime,time
import os
import re

def classify_data(data, pid, level=0):

    result = []

    for v in data:
        if v["pid"]==pid:
            v["level"] = level

            if "childs" not in v.keys():
                v["childs"]=[]

            inner_result = classify_data(data, v["id"], level + 1)

            if inner_result:
                for inner_v in inner_result:
                    v["childs"].append(inner_v)

            result.append(v)

    return result

def buildPageLabels(page,page_num):
    """
    :param page: 当前页面
    :param page_num: 总页数
    :return:
    返回式例：
        [{'page': 1, 'name': 1, 'cur': True}, {'page': 2, 'name': 2, 'cur': False}, {'page': 2, 'name': '下一页'}]

    """

    pageLabels = []
    if page > 1:
        pageLabels.append({
            "page": 1,
            "name": "首页"
        })
        pageLabels.append({
            "page": page - 1,  # 当前页点击时候触发的页数
            "name": "上一页"
        })
    if page == 1:
        pageArray = [1, 2, 3, 4]
    else:
        pageArray = list(range(page - 1, page + 3))  # page-1,page,page+1,page+2

    for p in pageArray:
        if p <= page_num:
            if page==p:
                cur = 1
            else:
                cur = 0
            pageLabels.append({
                "page": p,
                "name": p,
                "cur": cur
            })

    if page + 1 <= page_num:
        pageLabels.append({
            "page": page + 1,
            "name": "下一页"
        })

    return pageLabels

def GenFileDirs(path):
    purpose_path = os.path.join(path, time.strftime("%Y"))
    purpose_path = os.path.join(purpose_path, time.strftime("%m"))
    purpose_path = os.path.join(purpose_path, time.strftime("%d"))
    purpose_path = os.path.join(purpose_path, time.strftime("%H%M"))

    if not os.path.exists(purpose_path):
        os.makedirs(purpose_path)

    return purpose_path
def GenImageFileName(prefix='', suffix=''):
    r = "%d%d" % (random.randint(1000, 9999), random.randint(1000, 9999))
    return prefix+r+suffix

def gen_random_code_s(prefix):
    """
    产生随机编号（服务于数据表的的编号）
    :param prefix: 编码前缀
    :return:
    """
    val = str(uuid.uuid5(uuid.uuid1(), str(uuid.uuid1())))
    a = val.split("-")[0]
    code = "%s%s%d" % (prefix,a, random.randint(10000, 99999))

    return code

def gen_random_code(prefix):
    """
    产生永远不重复的随机数
    :param prefix: 编码前缀
    :return:
    """
    # d= self.get_datetime_format("%Y%m%d%H%M%S")
    d = time.strftime("%Y%m%d")

    val = str(uuid.uuid5(uuid.uuid1(), str(uuid.uuid1())))
    a = val.split("-")[0]
    code = "%s_%s_%s%d" % (prefix, d, a, random.randint(100, 999))

    return code


def gen_dateList_startAndEnd(start, end):
    start_date = datetime.date(*start)
    end_date = datetime.date(*end)

    result = []

    curr_date = start_date
    while curr_date != end_date:
        # t="%04d-%02d-%02d" % (curr_date.year, curr_date.month, curr_date.day)

        result.append({
            "ym": "%04d-%02d" % (curr_date.year, curr_date.month),
            "ymd": curr_date
        })

        curr_date += datetime.timedelta(1)
    # result.append("%04d%02d%02d" % (curr_date.year, curr_date.month, curr_date.day))

    return result


def validate_email(s):
    ex_email = re.compile(r'(^[\w][a-zA-Z0-9.]{4,19})@[a-zA-Z0-9]{2,3}.com')
    r = ex_email.match(s)

    if r:
        return True
    else:
        return False
def validate_tel(s):
    ex_tel = re.compile(r'(^[0-9\-]{11,15})')
    r = ex_tel.match(s)

    if r:
        return True
    else:
        return False