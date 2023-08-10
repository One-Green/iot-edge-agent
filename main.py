import os.path
from git import Repo
from fastapi import FastAPI
from fastapi.responses import FileResponse
from tempfile import TemporaryDirectory
from build_tool.models import BuildParam
from build_tool.logger import logger
from build_tool.build import (
    pio_build,
    pio_update_platform_ini,
    pio_generate_build_flags,
    make_zip
)
import time

app = FastAPI()


@app.post("/build")
def build(_build: BuildParam):
    logger.info(msg={
        "message": "cloning repo",
        "git_repo": _build.git_repo,
        "git_ref": _build.git_ref
    })
    with TemporaryDirectory() as _tmp:
        repo = Repo.clone_from(_build.git_repo, _tmp)
        repo.git.checkout(_build.git_ref)
        logger.debug(msg={"message": "cloned",
                          "tmp_folder": _tmp})

        src_path = os.path.join(_tmp, _build.src_path)

        if _build.platformio_build_flags:
            logger.debug({"platformio_build_flags": "Found"})
            flags = pio_generate_build_flags(_build.platformio_build_flags)
            logger.debug({"build_flags": flags})
            pio_update_platform_ini(src_path, flags)
        pio_build(src_path)
        dist_path = os.path.join(src_path, ".pio", "build")
        logger.debug({"dist_path": dist_path})
        time.sleep(60)
        target_zip_path, zip_file = make_zip(dist_path)
        logger.debug({"zip_path": str(target_zip_path)})

        return FileResponse(
            path=target_zip_path,
            media_type="application/x-zip-compressed",
            headers={"Content-Disposition": f"attachment; filename={zip_file}"}
        )


@app.get("/test")
def build():
    return FileResponse(
        path='/tmp/tmptalkxnnl2/sprinkler/.pio/build/dist-2ddc3778-37b5-11ee-b376-0242ac120002.zip',
        media_type="application/x-zip-compressed",
        headers={"Content-Disposition": f"attachment; filename=dist-2ddc3778-37b5-11ee-b376-0242ac120002.zip"}
    )
