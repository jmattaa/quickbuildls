# quickbuildls

a simple language server for [quickbuild](https://github.com/nordtechtiger/quickbuild)

### supports:

- completions & snippets
- hover
- diagnostics
- goto definition


simple neovim setup:

```lua
vim.api.nvim_create_autocmd({ "BufRead", "BufNewFile" }, {
    pattern = { "quickbuild" },
    callback = function(args)
        local clients = vim.lsp.get_clients({ name = "quickbuildls" })

        ---@type integer?
        local client_id = clients[1] and clients[1].id or nil
        if not client_id then
            client_id = vim.lsp.start({
                name = "quickbuildls",
                cmd = { "path/to/quickbuildls" },
            })
        end

        if not client_id then
            vim.notify("Failed to start quickbuildls", vim.log.levels.ERROR)
            return
        end

        local bufnr = args.buf
        -- lsp
        local ok = vim.lsp.buf_attach_client(bufnr, client_id)
        if ok then
            vim.notify("Attached quickbuildls", vim.log.levels.INFO)
        else
            vim.notify("Failed to attach quickbuildls", vim.log.levels.WARN)
        end
    end

    -- optional for filetype (mayebe syntax higlighting)
    vim.bo.filetype = "quickbuild"
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
