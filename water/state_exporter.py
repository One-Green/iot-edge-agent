import sys

sys.path.insert(0, "..")

import falcon
from waitress import serve
from core.db import get_state
import simplejson as json
from datetime import datetime


class StateView:
    def on_get(self, req, resp):
        """Handles GET requests"""
        d = get_state()
        d["utc_now"] = datetime.utcnow().isoformat()
        resp.body = json.dumps(d)
        resp.status = falcon.HTTP_200


api = falcon.API()
api.add_route('/state', StateView())

if __name__ == "__main__":
    serve(api, host='0.0.0.0', port=8080)
