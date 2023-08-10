from pydantic import BaseModel


class BuildParam(BaseModel):
    git_repo: str
    git_ref: str
    git_username: str | None
    git_password: str | None
    src_path: str
    platformio_build_flags: list | None
