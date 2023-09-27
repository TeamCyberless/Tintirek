package tools

import (
	"os"
)

func GetEnv(envName string, defValue string) string {
	value := os.Getenv(envName)
	if value != "" {
		return value
	}

	return defValue
}