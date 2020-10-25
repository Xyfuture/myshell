all: testsh msh testinput.txt
	@rm -rf tmp
	@mkdir tmp
	@cp ./testsh tmp
	@cp ./msh tmp
	@cp testinput.txt tmp

%: %.c
	@echo + gcc $<
	@gcc $< -o $@

grade:
	@echo + grading...
	@cd tmp && ./testsh ./msh

clean:
	-rm -rf tmp testsh msh
