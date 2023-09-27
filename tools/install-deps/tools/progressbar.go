package tools

import (
	"fmt"
	"strings"
	"sync"
)

type ProgressBar struct {
	mu				sync.Mutex
	Total			int64
	Current			int64
	BarSize			int
	CurrentSymbol	string
	EmptySymbol		string
}

func NewProgressBar(total int64, barSize int) *ProgressBar {
	return &ProgressBar {
		Total:			total,
		BarSize:		barSize,
		CurrentSymbol:	"=",
		EmptySymbol:	" ",
	}
}

func (p *ProgressBar) Update(current int64) {
	p.mu.Lock()
	defer p.mu.Unlock()
	p.Current = current
}

func (p *ProgressBar) Render() string {
	p.mu.Lock()
	defer p.mu.Unlock()

	progress := int(float64(p.Current) / float64(p.Total) * float64(p.BarSize))
	bar := strings.Repeat(p.CurrentSymbol, progress) + strings.Repeat(p.EmptySymbol, p.BarSize - progress)
	percent := (p.Current * 100) / p.Total
	return fmt.Sprintf("[%s] %3d%%", bar, percent)
}