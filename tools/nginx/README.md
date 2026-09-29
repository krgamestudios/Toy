Toylang's `docker-compose.yml` volumes should include:

```
- ./Toy/tools/nginx/nginx.conf:/etc/nginx/nginx.conf
- ./Toy/tools/nginx/toylang.conf:/etc/nginx/conf.d/default.conf
- ./Toy/docs/book:/usr/share/nginx/html
```


