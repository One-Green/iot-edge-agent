FROM python:3.11.0-bullseye
ENV PYTHONUNBUFFERED=true

WORKDIR /app

COPY build_tool/ ./build_tool/
COPY main.py .
COPY requirements.txt .

RUN apt update && apt install git -y
RUN pip install -r requirements.txt
RUN pio upgrade

ENTRYPOINT []
CMD uvicorn main:app --host=0.0.0.0 --port=8080 --reload
