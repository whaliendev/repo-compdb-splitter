package model

// CompileCommand specifies how a translation unit is compiled in the project.
// adapted from [compdb](https://pkg.go.dev/go.fuchsia.dev/fuchsia/tools/build/ninjago/compdb)
type CompileCommand struct {
	// The working directory of the compilation
	Directory string `json:"directory"`
	// The main translation unit source processed by this compilation step
	File string `json:"file"`
	// The compile command executed.
	Command string `json:"command"`
	// The compile command executed as list of strings
	Arguments []string `json:"arguments"`
	// The name of the output created by this compilation step
	Output string `json:"output"`
}
