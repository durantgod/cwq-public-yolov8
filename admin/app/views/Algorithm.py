from app.views.ViewsBase import *
from app.models import *
from django.shortcuts import render,redirect

def web_algorithms(request):
    context = {
        'data': get_algorithm_data
    }
    if request.method == "POST":
        print("-------------------------", request.POST)

        if 'add' in request.POST:
            code = request.POST.get('code', "").strip()
            name = request.POST.get('name', "").strip()
            objects = request.POST.get('objects', "").strip()
            remark = request.POST.get('remark', "").strip()
            print(code, name, objects, remark)

            if code == "" or name == "" or objects == "" or remark == "":
                print("添加算法参数不能为空")
            else:
                object_array = objects.split(",")
                if len(object_array) > 0:
                    object_count = len(object_array)

                    g_djangoSql.insert(tb_name="av_algorithm", d={
                        "sort": 0,
                        "code": code,
                        "name": name,
                        "object_count": object_count,
                        "objects": objects,
                        "remark": remark,
                        "state": 0
                    })
                    print("添加成功")
                else:
                    print("算法分类参数格式不正确")

            return redirect('/algorithms')

        if 'delete' in request.POST:
            algorithm_id = int(request.POST.get('algorithm_id'))
            try:
                algorithm = g_djangoSql.select("select 1 from av_algorithm where id=%d" % algorithm_id)
                if len(algorithm) > 0:
                    g_djangoSql.execute("delete from av_algorithm where id=%d" % algorithm_id)
                    print("删除成功")
                else:
                    print("删除算法不存在")
            except Exception as e:
                print("删除算法失败：", e)

            return redirect('/algorithms')

    return render(request, 'app/web_algorithms.html', context)

