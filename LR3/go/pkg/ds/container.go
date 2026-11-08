package ds

type Container interface {
	Len() int
	Clear()
	String() string
	Kind() string
}

func IsEmpty(container Container) bool {
	return container.Len() == 0
}
