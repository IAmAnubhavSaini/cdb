# cdb

Build a database from scratch in C. This is just for practice.

Build with Docker only. I had to put half a day to figure out how to build it in docker, not gonna use the peasant way anymore. (lol)

## Build with docker

```bash

# in your copy of the source code, in a terminal

docker build --progress=plain -t cdb-cmake-build .
docker run --rm -it -v `pwd`:/app cdb-cmake-build:latest

## once you are dropped in the container shell

cmake .
make


## Run the built binary with the database name

./build-db your-database-name

## # you are now in the database shell (marked by "db > ")
db > insert 10 "your-username" "your-email" "your-password"
db > select
db > .exit

```

## Build on bare metal

Use latest cmake, figure out stuff and raise a pull request.

```
cmake .
make
```

## Run tests

```
# build first
gem install --install-dir ./.bundle rspec
bundle exec rspec spec.rb
```

## Sample run

```bash

## your binary is ./build_db

./build_db sample-name # sample-name is the name of your database.

## you will be dropped in a database shell that the binary provides.
## it is marked by "db > "
db > 

## insert a record:
db > insert 1 "username" "email" "password"
db > select
db > .exit

## Run again, to see that the data is saved in the database
./build_db sample-name

db > select
# you should see your data here...

## use .exit to quit
db > .exit

```

## License

MIT

## Reference

[DB tutorial](https://cstack.github.io/db_tutorial/)
