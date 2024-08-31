from controllers.BaseHandler import BaseHandler
import json


class IndexHandler(BaseHandler):
    async def get(self, *args, **kwargs):
        data = await self.do()
        self.response_json(data)

    async def do(self):
        return "index"
