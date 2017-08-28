#pragma once

struct terminal
{
    terminal();
    ~terminal();
	operator bool() const;
    bool		isactive();
    bool		launch(const char* url);
    int			read(char* buffer, int count);
private:
    void		*child_out_rd, *child_out_wr;
    void		*child_in_rd, *child_in_wr;
    void		*child_proc, *child_thrd;
	unsigned	status;
};