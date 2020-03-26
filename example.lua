-- The special "nil" singleton value
what = nil

-- Booleans can be either "true" or "false"
nice = true

-- Integers of course
x = 42
print("x = " .. x)

-- Also (floating point) numbers
pi = 3.14159265

-- Strings for all your text needs
name = "Rhymu"

-- Functions are first-class objects in Lua!
sum = function(a, b)
    return a + b
end
print("2 + 3 = " .. sum(2, 3))

-- Tables can be lists...
primes = {2, 3, 5, 7, 11}

-- Tables can also be dictionaries, and the keys can be anything!
key = {1, 2, 3}
record = {
    job = "developer",
    [42] = "answer",
    [key] = "you found the list!",
}
print("record['job'] = '" .. record['job'] .. "'")
print("record[42] = '" .. record[42] .. "'")

-- Indexing is by reference, not value!
print("record[{1, 2, 3}] = " .. tostring(record[{1, 2, 3}]))
print("record[key] = '" .. record[key] .. "'")

-- Functions can return multiple return values!
function first_two(list)
    return list[1], list[2]
end

-- The "print" library function can deal with multiple arguments!
print("First two primes are:")
print(first_two(primes))
