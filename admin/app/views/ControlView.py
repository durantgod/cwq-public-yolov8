from app.views.ViewsBase import *
from app.models import *
from django.shortcuts import render,redirect
from app.utils.Utils import gen_random_code_s

def web_controls(request):
    context = {
    }

    return render(request, 'app/control/web_controls.html', context)


def web_add_control(request):
    context = {
    }
    algorithms_data = get_algorithm_data()

    streams = g_media.getMediaList()


    context["streams"] = streams
    context["algorithms"] = algorithms_data
    context["handle"] = "add"

    context["control"] = {
        "code": gen_random_code_s("control"),
        "min_interval": 180,
        "class_thresh": 0.5,
        "overlap_thresh": 0.5,
        "push_stream": True
    }

    return render(request, 'app/control/web_add_control.html', context)


def web_edit_control(request):
    context = {
    }
    params = parse_get_params(request)

    code = params.get("code")
    try:

        control = Control.objects.get(code=code)

        old_objects_data = []

        algorithms_data = get_algorithm_data()
        for algorithm in algorithms_data:
            if control.algorithm_code == algorithm["code"]:
                objects = algorithm["objects"]
                old_objects_data = objects.split(",")

        # context["streams"] = media.getStreams()
        context["algorithms"] = algorithms_data
        context["old_objects"] = old_objects_data
        context["handle"] = "edit"
        context["control"] = control
        context["control_stream_flvUrl"] = g_media.get_wsMp4Url(control.stream_app, control.stream_name)

    except Exception as e:
        print("web_control_edit error", e)

        return render(request, 'app/message.html',
                      {"msg": "请通过布控管理进入", "is_success": False, "redirect_url": "/controls"})

    return render(request, 'app/control/web_add_control.html', context)
