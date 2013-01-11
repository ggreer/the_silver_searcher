(require 'grep)

(defconst ag-buffer-name "*ag*")
(defvar ag-history-ring nil)
(defvar ag-binary "ag")
(defvar ag-default-options "--line-numbers")

(defun call-ag (commandline)
  (if (get-buffer ag-buffer-name)
      (kill-buffer ag-buffer-name))
  (let ((buf (get-buffer-create ag-buffer-name)))
    (switch-to-buffer buf)
    (shell-command commandline t)
    (if (= (point-min)
           (point-max))
        (progn
          (kill-buffer ag-buffer-name)
          (message "ag: No matches")
          nil)
      (progn
        (grep-mode)
        t))))

(defun ag-command()
  (concat ag-binary " " ag-default-options " "))

(defun ag (&optional pattern directory)
  (interactive)
  (unless directory
    (setq directory default-directory))
  (let ((command (if pattern
                     (concat (ag-command) pattern " " directory)
                   (read-from-minibuffer "Run: "
                                         (cons (concat (ag-command) " " directory)
                                               (+ (length (ag-command)) 1))
                                         nil
                                         nil
                                         'ag-history-ring))))
    (if (and command
             (not (string= command (concat (ag-command) " " directory))))
        (call-ag command))
    )
  )

(provide 'ag)
