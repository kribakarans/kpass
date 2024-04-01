# Kpass: command line password manager

### `kpass [flags] [options] ...`

### `Options:`
```
    -a  --add                   -- Add new password entry
    -e  --edit      <id>        -- Edit password entry of @id
    -r  --delete    <id>        -- Delete password entry of @id
    -g  --grep      <pattern>   -- Grep @pattern in the database
    -l  --list      <id>        -- List password entry of @id
    -f  --file      <database>  -- Use @file as current database
    -d  --debug                 -- Enable debug logging
    -i  --import    <database>  -- Import entries from the given file
    -E  --export    <csv-file>  -- Export entries to csv formatted file
    -v  --version               -- Print package version
    -h  --help                  -- Print this help menu
```
### `Usage:`
```
    kpass --add           -- Added new entry
    kpass --edit   1      -- Edit entry of @id 1
    kpass --delete 2      -- Delete entry 0f @id 2
    kpass --list          -- List all password entries
    kpass --list   3      -- List entry of @id 3
    kpass --grep   hello  -- Grep the string ''hello' in the database
```
