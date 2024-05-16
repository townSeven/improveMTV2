# improveMTV2

## Install

1. `mkdir build && cd build`

2. `cmake .. <option>`

    there are two optional version：

    * origin：`-DBUILD_TYPE=origin`
    * libpg：`-DBUILD_TYPE=libpg`

3. `make`

## Generate and load ssb

### Generate ssb data

`./tools/generate.sh ./resources/data/folder_name scale_factor`

### Load ssb into mysql

`mysql> source path/to/improveMTV2/resources/sql/initialize.sql`

## Use(libpg version)

`./ast <query> | ./libpg`

`<query>` 为 21 表示执行 Q2.1
