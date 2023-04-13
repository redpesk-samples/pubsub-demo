

# compile + launch

follow doc

https://docs.redpesk.bzh/docs/en/master/getting_started/host-configuration/docs/1-Setup-your-build-host.html

```bash
wget -O - https://raw.githubusercontent.com/redpesk-devtools/redpesk-sdk-tools/master/install-redpesk-sdk.sh | bash
```

# quick compilation + start
```
 ./start-demo.sh
```


# manual build/launch hackaton

```
# compile sample
mkdir -p build && cd build
cmake .. && make

# start-demo
afb-binder --config=hackaton.config -vvv
```
