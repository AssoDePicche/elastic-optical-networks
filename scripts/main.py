import sys

from scipy.special import factorial

from scipy.optimize import bisect

def erlang_b(grade_of_service, resources):
    return (grade_of_service**resources) / factorial(resources) / sum((grade_of_service**k) / factorial(k) for k in range(resources + 1))

def fn(grade_of_service, resources, desired_grade_of_service):
    return erlang_b(grade_of_service, resources) - desired_grade_of_service

if __name__ == '__main__':
  if len(sys.argv) != 3:
    print(f'You must inform the number of resources and desired grade of service')

    exit(1)

  try:
    resources = int(sys.argv[1])

    desired_grade_of_service = float(sys.argv[2])

    erlangs = bisect(fn, 0.1, 100, args=(resources, desired_grade_of_service))

    print(f"{erlangs:.4f} E")
  except Exception as exception:
    print(f'Error: {exception}')
