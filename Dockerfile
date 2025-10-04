FROM ubuntu:22.04
#ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y \
#    build-essential \
#    g++ \
#    make \
    #libmysqlclient-dev \
#    libmysqlcppconn-dev \
    libmysqlcppconn7v5 \
#    && ldconfig \
    #ca-certificates \
    && rm -rf /var/lib/apt/lists/*

RUN echo "/usr/lib/x86_64-linux-gnu" > /etc/ld.so.conf.d/mysqlcppconn.conf && ldconfig

RUN mkdir -p /app /config /var/log
COPY ./bin/QMonitor /app/QMonitor
COPY ./config/config.json /config/config.json

WORKDIR /app
EXPOSE 8080

ENTRYPOINT ["/app/QMonitor","../config/config.json"]