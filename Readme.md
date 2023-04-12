

# compile + launch

follow doc

https://docs.redpesk.bzh/docs/en/master/getting_started/host-configuration/docs/1-Setup-your-build-host.html

```bash
wget -O - https://raw.githubusercontent.com/redpesk-devtools/redpesk-sdk-tools/master/install-redpesk-sdk.sh | bash
```


# build/launch hackaton sender before: follow its own readme

```bash
afb-binder --binding=./release.so --port 4442 -vvv --ws-server=unix:/tmp/release &
afb-binder --binding=./shadow.so --port 4441 -vvv --ws-server=unix:/tmp/shadow &
```


```
cmake .
make
afb-binder --binding=./libcompare.so -vvv --ws-client=unix:/tmp/release --ws-client=unix:/tmp/shadow --port=11111
```
