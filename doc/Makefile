target = compilers.pdf
tex_files = compilers.tex
tex_main = compilers.tex

all: $(target)

$(target): $(tex_files)
	pdflatex $(tex_main)
	# Regenerate with toc
	pdflatex $(tex_main)
