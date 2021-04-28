#
# For the copyright information for this file, please search up the
# directory tree for the first COPYING file.
#

FROM node

RUN : \
  && export DEBIAN_FRONTEND=noninteractive \
  && apt-get -q -y update \
  && apt-get -q -y install gcc g++ make nettle-dev \
  && npm install -g node-gyp \
;

COPY pfss-*.tar.gz /

RUN : \
  && mkdir /pfss.tmp \
  && cd /pfss.tmp \
  && tar xf ../pfss-*.tar.gz \
  && mv * /pfss \
  && cd /pfss \
  && rmdir /pfss.tmp \
  && ./configure --prefix=/usr \
  && make \
  && make install \
  && ldconfig \
;

WORKDIR /pfss/src/node

CMD /bin/bash
