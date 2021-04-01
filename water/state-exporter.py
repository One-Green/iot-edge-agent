import falcon
from core.db import get_state


class StateView:
    def on_get(self, req, resp):
        """Handles GET requests"""
        resp.media = get_state()


api = falcon.API()
api.add_route('/state', StateView())
