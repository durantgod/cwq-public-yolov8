import uuid
import random
import time
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
                cur = True
            else:
                cur = False
            pageLabels.append({
                "page": p,
                "name": p,
                "cur":cur
            })

    if page + 1 <= page_num:
        pageLabels.append({
            "page": page + 1,
            "name": "下一页"
        })

    return pageLabels
def gen_random_code(prefix=""):
    """
    生产永远不重复的随机数
    :param prefix: 编码前缀
    :return:
    """
    # d= self.get_datetime_format("%Y%m%d%H%M%S")
    d = time.strftime("%Y%m%d%H%M%S")

    val = str(uuid.uuid5(uuid.uuid1(), str(uuid.uuid1())))
    a = val.split("-")[0]
    code = "%s_%s_%s%d" % (prefix, d, a, random.randint(1000, 9999))

    return code
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