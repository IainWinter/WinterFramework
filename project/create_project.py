# Create a new Winter framework project using the template

import os

project_name = input("Enter the project name: ")
project_dir = input("Enter the project directory (default: current directory): ")
cmake_template = input("Enter the cmake template (default: template_CMakeLists.txt): ")
main_template = input("Enter the main.cpp template (default: template_main.cpp): ")
where_is_wf_installed = input("Winter Framework is installed in (default: ../): ")

if project_dir == "":
    project_dir = os.getcwd()

if where_is_wf_installed == "":
    where_is_wf_installed = "../"

if cmake_template == "":
    cmake_template = "./template_CMakeLists.txt"
    
if main_template == "":
    main_template = "./template_main.cpp"

project_dir = f'{project_dir}/{project_name}'
where_is_wf_installed = os.path.abspath(where_is_wf_installed).replace('\\', '/')

def write_templated_file(template: str, outfile: str):
	template_file = open(template, "r")
	outfile_file = open(outfile, "w")

	for line in template_file:
		outfile_file.write(
            line.replace("PROJECT_NAME", project_name)
                .replace("WINTER_FRAMEWORK_INSTALL_DIR", where_is_wf_installed))

	template_file.close()
	outfile_file.close()
    
os.mkdir(project_dir)
os.mkdir(project_dir + "/src")
os.mkdir(project_dir + "/build")

write_templated_file(cmake_template, f'{project_dir}/CMakeLists.txt')
write_templated_file(main_template, f'{project_dir}/src/main.cpp')

print(f'Project {project_name} created successfully!')
print(f'Project directory: {project_dir}')
print(f'Winter Framework install directory: {where_is_wf_installed}')
print(f'Run "cmake ../" in {project_dir}/build to generate the project files')