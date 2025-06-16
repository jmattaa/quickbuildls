# quickbuildls

a simple language server for [quickbuild](https://github.com/nordtechtiger/quickbuild)

### supports:

- completions & snippets
- hover
- diagnostics
- goto definition


simple neovim setup:

```lua
local client = vim.lsp.start_client({
    name = "quickbuildls",
    cmd = { "path/to/quickbuildls" },
})

if not client then
    vim.notify("Failed to start quickbuildls", vim.log.levels.ERROR)
    return
end

vim.api.nvim_create_autocmd({ "BufRead", "BufNewFile" }, {
    pattern = { "quickbuild" },
    callback = function(args)
        local bufnr = args.buf
        local ok = vim.lsp.buf_attach_client(bufnr, client)
        if ok then
            vim.notify("Attached quickbuildls", vim.log.levels.INFO)
       else
            vim.notify("Failed to attach quickbuildls", vim.log.levels.WARN)
        end
    end
})
```


btw this is pretty useful for debugging

```bash
# redirect stderr to a file
quickbuildls 2>> quickbuildls.log
```

or this is a pretty nice trick 
```bash
quickbuildls | tee quickbuildls.log 2> >(tee -a quickbuildls.log >&2)
```
