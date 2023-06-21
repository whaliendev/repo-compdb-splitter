package trie

type Node struct {
	children  map[string]*Node
	isWordEnd bool
}

type Trie struct {
	root *Node
}

func NewTree() *Trie {
	return &Trie{root: &Node{}}
}

func (t *Trie) Insert(words []string) {
	//current := t.root
	for i := 0; i < len(words); i++ {
	}
}

func (t *Trie) Find(words []string) bool {
	return false
}
