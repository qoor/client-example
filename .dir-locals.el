((c-mode . ((eval add-hook 'before-save-hook #'lsp-format-buffer nil t)
            (c-basic-offset . 2)))
 (c++-mode . ((eval add-hook 'before-save-hook #'lsp-format-buffer nil t)
              (c-basic-offset . 2))))
