# Micro

## Installation

Put the [`micro/toy.yaml`](micro/toy.yaml) file into the `/path/to/micro/config/syntax` directory (typically `$XDG_CONFIG_HOME/micro/syntax` or `$HOME/.config/micro/syntax`).

## Testing

1. Open a file with the `.toy` extension

2. Press `Ctrl+E`, type `show filetype`, and press `Enter`. `toy` must be shown

# Nano

## Installation

1. Put the [`nano/toy.nanorc`](nano/toy.nanorc) file into any desired place (ex.: `/.nano/toy.nanorc`)

2. Include it into your `.nanorc` file via the `include` directive (ex.: `include "/.nano/toy.nanorc"`)

> [!NOTE]
> This is not recommended, but you can directly insert the content of the file into your `.nanorc` for quick testings.

## Testing

Type a literal (ex.: `"Hello, World!"`) or a keyword (ex.: `return`) to see if it is highlighted.

# Vim

## Installation

1. Put the [`vim/toy.vim`](vim/toy.vim) file into the syntax file directory (ex.: `/.vim/syntax/`)

2. Add the following code into your filetype file (ex.: `/.vim/filetype.vim`):
```bash
augroup filetype_toy
  autocmd!
  autocmd BufRead,BufNewFile *.toy set filetype=toy
augroup END
```

## Testing

1. Go into the _NORMAL mode_ (press `Esc` while a `*.toy` file is opened if needed)

2. Type `:set filetype?` and press `Enter`. `toy` must be shown

## Notes

This instruction is also applicable for Neovim. However, the directories are going to be different from the example ones.
